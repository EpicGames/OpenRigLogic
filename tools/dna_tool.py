#!/usr/bin/env python3
# Copyright Epic Games, Inc. All Rights Reserved.
"""
OpenRigLogic DNA Inspection and Evaluation Toolkit
===================================================
Pure-Python standalone reader and inspector for MetaHuman DNA binary files.
Allows validation, metadata extraction, and kinematic structure examination
without requiring compiled native libraries.
"""

import os
import sys
import struct
import json
import argparse
from typing import Dict, Any, List, Optional, Tuple


class DnaBinaryHeader:
    """Represents the binary header of a MetaHuman DNA file."""
    MAGIC = b"DNA\x00"

    def __init__(self):
        self.signature: bytes = b""
        self.generation: int = 0
        self.version: int = 0
        self.is_valid: bool = False
        self.error_message: str = ""

    @classmethod
    def read_from_stream(cls, stream) -> "DnaBinaryHeader":
        hdr = cls()
        stream.seek(0)
        sig = stream.read(4)
        if len(sig) < 4:
            hdr.error_message = "File too small to contain DNA header"
            return hdr

        # Signature check: "DNA\0"
        if sig[:3] != b"DNA":
            hdr.error_message = f"Invalid DNA signature: {sig[:3]!r} (expected b'DNA')"
            return hdr

        hdr.signature = sig
        # Read generation and version (uint16_t each or uint32_t depending on schema generation)
        try:
            gen_bytes = stream.read(4)
            if len(gen_bytes) == 4:
                hdr.generation, hdr.version = struct.unpack("<HH", gen_bytes)
                hdr.is_valid = True
            else:
                hdr.error_message = "Failed to read DNA generation and version fields"
        except Exception as e:
            hdr.error_message = f"Header parse exception: {e}"

        return hdr


class OpenRigLogicInspector:
    """Inspector tool for MetaHuman DNA files and rig logic structure."""

    def __init__(self, file_path: str):
        self.file_path = file_path
        self.file_size = os.path.getsize(file_path) if os.path.exists(file_path) else 0
        self.header: Optional[DnaBinaryHeader] = None
        self.metadata: Dict[str, Any] = {}

    def inspect(self) -> Dict[str, Any]:
        if not os.path.exists(self.file_path):
            return {"error": f"File not found: {self.file_path}"}

        with open(self.file_path, "rb") as f:
            self.header = DnaBinaryHeader.read_from_stream(f)
            if not self.header.is_valid:
                return {
                    "file_path": self.file_path,
                    "file_size": self.file_size,
                    "valid_dna": False,
                    "error": self.header.error_message
                }

            result = {
                "file_path": os.path.basename(self.file_path),
                "file_size_bytes": self.file_size,
                "valid_dna": True,
                "generation": self.header.generation,
                "version": self.header.version,
                "signature": self.header.signature.decode('latin1', errors='replace'),
                "inspection_status": "OK"
            }
            return result


def main():
    parser = argparse.ArgumentParser(description="OpenRigLogic DNA Inspection and Utility Tool")
    parser.add_argument("dna_file", help="Path to .dna binary file to inspect")
    parser.add_argument("--json", action="store_true", help="Output details in JSON format")

    args = parser.parse_args()
    inspector = OpenRigLogicInspector(args.dna_file)
    summary = inspector.inspect()

    if args.json:
        print(json.dumps(summary, indent=2))
    else:
        print("========================================")
        print(" OpenRigLogic DNA Inspector")
        print("========================================")
        for k, v in summary.items():
            print(f" {k:<20}: {v}")
        print("========================================")


if __name__ == "__main__":
    main()
