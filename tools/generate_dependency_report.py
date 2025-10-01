#!/usr/bin/env python3
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Dict, Optional, Tuple

REPO_ROOT = Path(__file__).resolve().parents[1]
DOCS_DIR = REPO_ROOT / "docs" / "dependencies"


@dataclass
class Dep:
    name: str
    version: Optional[str] = None
    source_file: Optional[Path] = None
    extra: Optional[str] = None


def debug(msg: str):
    # Simple toggleable debug logger
    if os.environ.get("DEP_REPORT_DEBUG"):
        print(f"[DEBUG] {msg}")


def read_text_safe(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except Exception:
        return ""


def parse_requirements_line(line: str) -> Tuple[str, Optional[str]]:
    line = line.strip()
    if not line or line.startswith("#") or line.startswith("-r ") or line.startswith("--"):
        return ("", None)
    # Handle extras and environment markers loosely
    # examples: package==1.2.3; python_version>='3.8'
    #           package>=1.2
    #           package
    part = line.split(";")[0].strip()
    # Remove extras in brackets for name
    m = re.match(r"^([A-Za-z0-9_.\-]+)(\[.*?\])?(==|>=|<=|>|<|~=)?(.*)?$", part)
    if not m:
        return (part, None)
    name = m.group(1) or part
    op = m.group(3)
    ver = (m.group(4) or '').strip()
    version = f"{op}{ver}" if op and ver else (ver if ver else None)
    return (name, version)


def collect_python_requirements() -> List[Dep]:
    deps: List[Dep] = []
    req_files = [
        REPO_ROOT / "requirements.txt",
        REPO_ROOT / "requirements-tests.txt",
        REPO_ROOT / "requirements-doc.txt",
        REPO_ROOT / "tests" / "end_to_end" / "benders" / "requirements.txt",
        REPO_ROOT / "tests" / "end_to_end" / "examples" / "requirements.txt",
    ]
    # Deduplicate while preserving first occurrence source
    seen = {}
    for rf in req_files:
        if rf.exists():
            for line in read_text_safe(rf).splitlines():
                name, ver = parse_requirements_line(line)
                if not name:
                    continue
                key = name.lower()
                if key not in seen:
                    seen[key] = Dep(name=name, version=ver, source_file=rf)
                else:
                    # Prefer pinned version if available
                    if ver and (not seen[key].version or '>' in seen[key].version):
                        seen[key].version = ver
    # environment.yml (conda)
    env_yml = REPO_ROOT / "environment.yml"
    if env_yml.exists():
        for line in read_text_safe(env_yml).splitlines():
            if re.match(r"^\s*#", line):
                continue
            m = re.match(r"^\s*-\s*([A-Za-z0-9_.\-]+)([=><!~].*)?$", line)
            if m:
                name = m.group(1)
                ver = (m.group(2) or '').strip()
                key = name.lower()
                if key not in seen:
                    seen[key] = Dep(name=name, version=ver or None, source_file=env_yml)
                else:
                    if ver and not seen[key].version:
                        seen[key].version = ver
    deps.extend(seen.values())
    return deps


def collect_vcpkg_dependencies() -> Tuple[List[Dep], List[Dep]]:
    vcpkg_manifest = REPO_ROOT / "vcpkg.json"
    deps: List[Dep] = []
    overrides: List[Dep] = []
    if not vcpkg_manifest.exists():
        return deps, overrides
    try:
        data = json.loads(vcpkg_manifest.read_text(encoding="utf-8"))
    except Exception as e:
        debug(f"Failed to parse vcpkg.json: {e}")
        return deps, overrides
    for dep in data.get("dependencies", []):
        if isinstance(dep, str):
            deps.append(Dep(name=dep, version=None, source_file=vcpkg_manifest))
        elif isinstance(dep, dict):
            name = dep.get("name") or ""
            version = dep.get("version>=") or dep.get("version<") or dep.get("version")
            deps.append(Dep(name=name, version=version, source_file=vcpkg_manifest, extra=json.dumps(
                {k: v for k, v in dep.items() if k not in {"name", "version>=", "version<", "version"}},
                ensure_ascii=False)))
    for ov in data.get("overrides", []):
        name = ov.get("name") or ""
        version = ov.get("version") or ov.get("version-string") or ov.get("version>=")
        overrides.append(Dep(name=name, version=version, source_file=vcpkg_manifest))
    return deps, overrides


def collect_cmake_dependencies() -> List[Dep]:
    cmake_files: List[Path] = []
    for root, dirs, files in os.walk(REPO_ROOT):
        # Skip vcpkg, build dirs, and vendored/docker directories
        if any(skip in root for skip in
               ["/vcpkg/", "/vcpkg_installed/", "/build_", "/.git/", "/docker/buildx/vendor/"]):
            continue
        for f in files:
            if f == "CMakeLists.txt" or f.endswith(".cmake"):
                cmake_files.append(Path(root) / f)
    pattern_find = re.compile(r"^\s*find_package\s*\(\s*([A-Za-z0-9_\-]+)\s*([^)]*)\)", re.IGNORECASE)
    pattern_fetch = re.compile(r"^\s*FetchContent_Declare\s*\(\s*([A-Za-z0-9_\-]+)", re.IGNORECASE)
    deps: Dict[str, Dep] = {}
    for cf in cmake_files:
        for line in read_text_safe(cf).splitlines():
            m = pattern_find.search(line)
            if m:
                name = m.group(1)
                args = (m.group(2) or '').strip()
                ver = None
                # Try to catch VERSION or exact version numbers in args
                vm = re.search(r"(VERSION|[0-9]+\.[0-9]+(\.[0-9]+)?)", args)
                if vm:
                    ver = vm.group(0) if vm.group(1) != 'VERSION' else None
                key = name.lower()
                if key not in deps:
                    deps[key] = Dep(name=name, version=ver, source_file=cf)
            fm = pattern_fetch.search(line)
            if fm:
                name = fm.group(1)
                key = f"fetchcontent:{name.lower()}"
                if key not in deps:
                    deps[key] = Dep(name=name, version=None, source_file=cf, extra="FetchContent")
    return list(deps.values())


def collect_github_workflow_dependencies() -> List[Dep]:
    deps: List[Dep] = []
    wf_dir = REPO_ROOT / ".github" / "workflows"
    if not wf_dir.exists():
        return deps
    for yml in sorted(wf_dir.glob("*.yml")) + sorted(wf_dir.glob("*.yaml")):
        content = read_text_safe(yml)
        for line in content.splitlines():
            line_s = line.strip()
            if line_s.startswith("uses:"):
                # e.g., uses: actions/checkout@v4
                val = line_s.split(":", 1)[1].strip()
                m = re.match(r"([A-Za-z0-9_./\-]+)@([A-Za-z0-9_./\-]+)", val)
                if m:
                    deps.append(Dep(name=m.group(1), version=m.group(2), source_file=yml))
                else:
                    deps.append(Dep(name=val, version=None, source_file=yml))
            if "pip install" in line_s:
                # crude extraction: pip install pkg==ver
                pkgs = re.findall(r"pip install ([^#]+)$", line_s)
                for p in pkgs:
                    for token in p.split():
                        name, ver = parse_requirements_line(token)
                        if name:
                            deps.append(Dep(name=name, version=ver, source_file=yml, extra="workflow:pip"))
            if re.search(r"apt(-get)?\s+install", line_s):
                # apt install packages
                m = re.search(r"install\s+(-y\s+)?(.+)$", line_s)
                if m:
                    for token in m.group(2).split():
                        if token.startswith("-"):
                            continue
                        deps.append(Dep(name=token, version=None, source_file=yml, extra="workflow:apt"))
    return deps


def find_vcpkg_exe() -> Optional[Path]:
    # Prefer local vcpkg if present
    candidates = [
        REPO_ROOT / "vcpkg" / "vcpkg",
        Path(os.environ.get("VCPKG_ROOT", "")) / "vcpkg" if os.environ.get("VCPKG_ROOT") else None,
        Path("vcpkg"),
    ]
    for exe in candidates:
        if exe and exe.exists() and os.access(exe, os.X_OK):
            return exe
    return None


def _filter_vcpkg_tree(text: str) -> str:
    """Filter out noise lines from vcpkg dependency tree output.
    Removes any line mentioning vcpkg-cmake or vcpkg-cmake-config.
    """
    lines = text.splitlines()
    filtered = [ln for ln in lines if ("vcpkg-cmake" not in ln and "vcpkg-cmake-config" not in ln)]
    return "\n".join(filtered)


def run_vcpkg_depend_info_for(packages: List[str]) -> Dict[str, str]:
    """Run `vcpkg depend-info --format=tree <package>` for each given package name.
    Returns a mapping from package name to its dependency tree text.
    """
    exe = find_vcpkg_exe()
    if not exe:
        return {}
    results: Dict[str, str] = {}
    # Deduplicate and sort for stable output
    for pkg in sorted({p.strip() for p in packages if p and p.strip()}):
        try:
            debug(f"Running: {exe} depend-info --format=tree {pkg} (cwd={REPO_ROOT})")
            out = subprocess.check_output(
                [str(exe), "depend-info", "--format=tree", pkg],
                cwd=str(REPO_ROOT),
                stderr=subprocess.STDOUT,
                timeout=120,
            )
            raw = out.decode("utf-8", errors="replace")
            results[pkg] = _filter_vcpkg_tree(raw)
        except subprocess.CalledProcessError as e:
            debug(f"vcpkg depend-info failed for {pkg}: {e.output.decode(errors='replace')}")
        except Exception as e:
            debug(f"vcpkg depend-info error for {pkg}: {e}")
    return results


def ensure_docs_dir():
    DOCS_DIR.mkdir(parents=True, exist_ok=True)


def to_markdown_table(rows: List[Tuple[str, str, str, str]]) -> str:
    # rows: (Name, Version, Source, Rationale placeholders)
    lines = []
    lines.append("| Dependency | Version/Constraint | Source | Rationale (Why needed) | Rationale (Why this version) |")
    lines.append("|---|---|---|---|---|")
    for name, version, source, rationale in rows:
        lines.append(f"| {name} | {version or ''} | {source or ''} |  |  |")
    return "\n".join(lines)


def _dedupe_and_rows(deps: List[Dep], include_extra_in_key: bool = True) -> List[Tuple[str, str, str, str]]:
    # Group by (name, version, [extra]) and aggregate unique sources
    groups: Dict[Tuple[str, str, Optional[str]], Dict[str, object]] = {}
    for d in deps:
        key = (d.name, d.version or "", (d.extra or "") if include_extra_in_key else "")
        entry = groups.get(key)
        rel_source = str(d.source_file.relative_to(REPO_ROOT)) if d.source_file else ""
        if not entry:
            groups[key] = {
                "name": d.name,
                "version": d.version or "",
                "extra": d.extra or "",
                "sources": set([rel_source]) if rel_source else set(),
            }
        else:
            if rel_source:
                entry["sources"].add(rel_source)
    # Prepare rows with sorted unique sources joined by ", "
    rows: List[Tuple[str, str, str, str]] = []
    for (_, _, _), data in sorted(groups.items(), key=lambda kv: (kv[0][0].lower(), kv[0][1])):
        sources_joined = ", ".join(sorted(data["sources"]))
        rows.append((data["name"], data["version"], sources_joined, data["extra"]))
    return rows


def generate_report():
    ensure_docs_dir()

    py_deps = collect_python_requirements()
    vcpkg_deps, vcpkg_overrides = collect_vcpkg_dependencies()
    cmake_deps = collect_cmake_dependencies()
    gh_deps = collect_github_workflow_dependencies()

    # VCPKG dependency trees per package
    trees = run_vcpkg_depend_info_for([d.name for d in vcpkg_deps if d.name])
    tree_path = None
    combined_tree_text = None
    if trees:
        # Combine with clear headers
        lines: List[str] = []
        for pkg in sorted(trees.keys(), key=lambda s: s.lower()):
            lines.append(f"===== {pkg} =====")
            text = (trees.get(pkg) or "").strip()
            if text:
                lines.extend(text.splitlines())
            lines.append("")
        combined_tree_text = "\n".join(lines).strip() + "\n"
        tree_path = DOCS_DIR / "vcpkg_dependency_tree.txt"
        tree_path.write_text(combined_tree_text, encoding="utf-8")

    # Prepare markdown
    md_lines: List[str] = []
    md_lines.append("# Project Dependency Report")
    md_lines.append("")
    md_lines.append(
        f"Generated by tools/generate_dependency_report.py on {__import__('datetime').datetime.now().isoformat(timespec='seconds')}.")
    md_lines.append("")

    md_lines.append("## Python runtime/test/doc dependencies")
    py_rows = _dedupe_and_rows(py_deps, include_extra_in_key=False)
    if py_rows:
        md_lines.append(to_markdown_table(py_rows))
    else:
        md_lines.append("No Python dependency files were found.")
    md_lines.append("")

    md_lines.append("## CMake build dependencies (find_package/FetchContent)")
    cm_rows = _dedupe_and_rows(cmake_deps, include_extra_in_key=True)
    if cm_rows:
        md_lines.append(to_markdown_table(cm_rows))
    else:
        md_lines.append("No CMake dependencies detected.")
    md_lines.append("")

    md_lines.append("## vcpkg manifest dependencies")
    vc_rows = _dedupe_and_rows(vcpkg_deps, include_extra_in_key=True)
    if vc_rows:
        md_lines.append(to_markdown_table(vc_rows))
    else:
        md_lines.append("No vcpkg manifest found.")
    md_lines.append("")

    md_lines.append("### vcpkg overrides (pinning)")
    ov_rows = _dedupe_and_rows(vcpkg_overrides, include_extra_in_key=False)
    if ov_rows:
        md_lines.append(to_markdown_table(ov_rows))
    else:
        md_lines.append("No vcpkg overrides detected.")
    md_lines.append("")

    # vcpkg tree subsection under vcpkg deps
    md_lines.append("### vcpkg dependency tree")
    md_lines.append("Note: In the trees below, the placeholder '...' indicates a repeated dependency subtree already shown earlier.")
    if tree_path and combined_tree_text:
        md_lines.append(
            f"The per-package dependency trees are embedded below, and also saved at: `{tree_path.relative_to(REPO_ROOT)}`.")
        md_lines.append("")
        md_lines.append("```text")
        md_lines.extend(combined_tree_text.strip().splitlines())
        md_lines.append("```")
    else:
        md_lines.append(
            "vcpkg was not found or 'vcpkg depend-info' could not be executed. To include the tree, ensure vcpkg is available at ./vcpkg/vcpkg or set $VCPKG_ROOT, then rerun the generator.")
    md_lines.append("")

    md_lines.append("## GitHub workflow dependencies")
    gh_rows = _dedupe_and_rows(gh_deps, include_extra_in_key=True)
    if gh_rows:
        md_lines.append(to_markdown_table(gh_rows))
    else:
        md_lines.append("No GitHub workflows found.")
    md_lines.append("")

    md_lines.append("---")
    md_lines.append("How to regenerate this report:")
    md_lines.append("1. Ensure vcpkg is available (optional) at ./vcpkg/vcpkg or in $VCPKG_ROOT.")
    md_lines.append("2. Run: `python3 tools/generate_dependency_report.py` from the repository root.")

    out_path = DOCS_DIR / "dependency_report.md"
    out_path.write_text("\n".join(md_lines), encoding="utf-8")

    print(f"Dependency report written to {out_path}")
    if tree_path:
        print(f"vcpkg dependency tree written to {tree_path}")


if __name__ == "__main__":
    try:
        generate_report()
    except Exception as e:
        print(f"ERROR: Failed to generate dependency report: {e}", file=sys.stderr)
        sys.exit(1)
