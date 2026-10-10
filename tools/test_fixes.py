#!/usr/bin/env python3
# Copyright Epic Games, Inc. All Rights Reserved.
"""
Verification suite for OpenRigLogic fixes and enhancements.
"""

import os
import sys
import unittest
import tempfile
import io

# Test dna_tool
from dna_tool import DnaBinaryHeader, OpenRigLogicInspector

class TestOpenRigLogicFixes(unittest.TestCase):

    def test_dna_header_valid_signature(self):
        # Construct valid 8-byte DNA header: 'DNA\0', generation=2, version=1
        data = b"DNA\x00\x02\x00\x01\x00"
        stream = io.BytesIO(data)
        hdr = DnaBinaryHeader.read_from_stream(stream)
        self.assertTrue(hdr.is_valid)
        self.assertEqual(hdr.generation, 2)
        self.assertEqual(hdr.version, 1)

    def test_dna_header_invalid_signature(self):
        data = b"NOT_DNA_DATA"
        stream = io.BytesIO(data)
        hdr = DnaBinaryHeader.read_from_stream(stream)
        self.assertFalse(hdr.is_valid)
        self.assertIn("Invalid DNA signature", hdr.error_message)

    def test_dna_inspector_file(self):
        with tempfile.NamedTemporaryFile(suffix=".dna", delete=False) as tmp:
            tmp.write(b"DNA\x00\x02\x00\x01\x00dummy_payload_content")
            tmp_path = tmp.name

        try:
            inspector = OpenRigLogicInspector(tmp_path)
            res = inspector.inspect()
            self.assertTrue(res.get("valid_dna"))
            self.assertEqual(res.get("generation"), 2)
            self.assertEqual(res.get("version"), 1)
        finally:
            if os.path.exists(tmp_path):
                os.remove(tmp_path)

    def test_insert_copyright_encoding(self):
        # Verify utf-8 encoding in insert_copyright doesn't fail on unicode
        test_code = "# Unicode test: Çöğüşı € \nimport os\n"
        with tempfile.NamedTemporaryFile(suffix=".py", delete=False, mode="w", encoding="utf-8") as tmp:
            tmp.write(test_code)
            tmp_path = tmp.name

        try:
            import subprocess
            res = subprocess.run([sys.executable, "python/dna/insert_copyright.py", tmp_path],
                                 capture_output=True, text=True)
            self.assertEqual(res.returncode, 0)
            with open(tmp_path, "r", encoding="utf-8") as f:
                content = f.read()
            self.assertTrue(content.startswith("# Copyright Epic Games, Inc."))
            self.assertIn("Çöğüşı", content)
        finally:
            if os.path.exists(tmp_path):
                os.remove(tmp_path)


if __name__ == '__main__':
    unittest.main()
