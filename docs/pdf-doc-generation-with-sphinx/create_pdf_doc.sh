#!/bin/bash
# copy reference guide md files and assets
cp -r ../user-guide source/
cp -r ../assets/ source/
cp ../index.md source/user-guide/
# change style for inline latex math \\( -> $ and \\) -> $
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\\\)=$=g ; s=\\\\(=$=g' {} \;

# Remove badges
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\[!\[Status\].*\]\[.*\]==g; s=!\[C++\]\(.*\)==g; s=!\[Python\]\(.*\)==g' {} \;

# Replace \\{ with \{ 
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\\\{=\\\{=g ; s=\\\\}=\\\}=g' {} \;

# Replace \\\\ with \\
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\\\\\\\=\\\\=g' {} \;

# Replace \_ (used in mkdocs to avoid confusion with italic) with _ for latex
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\_=_=g' {} \;

# Replace [@bib_id] with \cite{bib_id} for latex
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\[@
\[@([A-Za-z0-9]+(_[A-Za-z0-9]+)+)\]
\]=\\cite\{$1\}=g' {} \;

# actually make the pdf
sphinx-build -M latexpdf source build
mv build/latex/antaresxpansionuserguide.pdf .
# clean
# rm -rf source/user-guide source/assets build
