import os
import sys

import pyximport

ROOT = os.getcwd()
BUILD = os.path.join(ROOT, "bindings", "python_cython", "build")
os.makedirs(BUILD, exist_ok=True)

pyximport.install(build_dir=BUILD)
sys.path.insert(0, os.path.join(ROOT, "bindings", "python_cython"))

from _cex_demo_cython import (  # noqa: E402
    CexDemoArgumentError,
    Request,
    run,
    traceback,
)


def main():
    req = Request(41)
    run(req)
    if req.value != 42:
        print(f"run failed: value={req.value}", file=sys.stderr)
        return 1
    print(f"value={req.value}")

    try:
        run(None)
    except CexDemoArgumentError as exc:
        print(f"error={exc}")
    else:
        print("NULL request returned unexpected error", file=sys.stderr)
        return 1

    tb = traceback()
    if "cex_demo__validate_request" not in tb or "cex_demo_run" not in tb:
        print("traceback missing frames", file=sys.stderr)
        return 1
    print(f"traceback={tb}", end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
