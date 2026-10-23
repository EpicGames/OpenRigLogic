# Copyright Epic Games, Inc. All Rights Reserved.
#-*- coding: utf-8 -*-

import argparse

def main():
    copyright_notice = "# Copyright Epic Games, Inc. All Rights Reserved.\n"

    parser = argparse.ArgumentParser(description="Inserts copyright notice at the start of the file.")
    parser.add_argument('file_path',
                        metavar='file_path',
                        help='File in which to insert the copyright notice.')

    args = parser.parse_args()
    input_file_path = args.file_path

    with open(input_file_path, 'r', encoding='utf-8') as input_file:
        first_line = input_file.readline()

        if copyright_notice not in first_line:
            input_file.seek(0)
            data = input_file.read()
            with open(input_file_path, 'w', encoding='utf-8') as output_file:
                output_file.write(copyright_notice + data)

if __name__ == '__main__':
    main()
