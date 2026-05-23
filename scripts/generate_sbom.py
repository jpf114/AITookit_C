#!/usr/bin/env python3
"""Generate a minimal SBOM (CycloneDX-style JSON) for AIToolkit release artifacts."""

import json
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

COMPONENTS = [
    {"name": "ai-toolkit-c", "version": "1.0.0", "type": "application", "purl": "pkg:github/AIToolkit/ai-toolkit-c@1.0.0"},
    {"name": "qtbase", "version": "6.x", "type": "library", "purl": "pkg:generic/qtbase@6"},
    {"name": "onnxruntime", "version": "1.x", "type": "library", "purl": "pkg:generic/onnxruntime@1"},
    {"name": "opencv4", "version": "4.x", "type": "library", "purl": "pkg:generic/opencv@4"},
]


def main() -> int:
    output = ROOT / "sbom.json"
    if len(sys.argv) > 1:
        output = Path(sys.argv[1])

    sbom = {
        "bomFormat": "CycloneDX",
        "specVersion": "1.4",
        "version": 1,
        "metadata": {
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "component": {
                "type": "application",
                "name": "AIToolkit",
                "version": "1.0.1",
            },
        },
        "components": [
            {
                "type": component["type"],
                "name": component["name"],
                "version": component["version"],
                "purl": component["purl"],
            }
            for component in COMPONENTS
        ],
    }

    output.write_text(json.dumps(sbom, indent=2), encoding="utf-8")
    print(f"SBOM written to {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
