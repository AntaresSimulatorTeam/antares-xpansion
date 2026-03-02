def get_results_file_path_from_logs(logs: bytes) -> str:
    for line in logs.splitlines():
        if b'Optimization results available in : ' in line:
            return line.split(b'Optimization results available in : ')[1].decode('ascii')
    raise LookupError("Could not find results file path in output logs")
