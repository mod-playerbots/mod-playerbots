"""Compile the real Zeliek center decision and fixed-point helper against a small movement fixture."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile


def function(source, signature, indent):
    start = source.index(indent + signature)
    end = source.index("\n" + indent + "}\n", start)
    return source[start:end + len(indent) + 2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[2])
    args = parser.parse_args()
    naxx = args.source / "src/Ai/Raid/Naxx"
    movement = function((naxx / "FourHorsemen40.cpp").read_text(), "bool MoveThroughCenter()", "    ")
    hazards = function((naxx / "FourHorsemen40Hazards.cpp").read_text(),
                       "bool FourHorsemen40Hazards::WalkToFixedPoint(", "")
    fixture = Path(__file__).with_name("zeliek_center_fixture.hpp").read_text()
    with tempfile.TemporaryDirectory() as temp:
        source = Path(temp) / "zeliek_center.cpp"
        binary = Path(temp) / "zeliek_center"
        source.write_text(fixture.replace("// CENTER_DECISION", movement).replace("// FIXED_POINT", hazards))
        subprocess.run([os.environ.get("CXX", "c++"), "-std=c++20", str(source), "-o", str(binary)], check=True)
        return subprocess.run([str(binary)], check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
