import sys

if __name__ == "__main__":
    readelf_output: str = sys.stdin.read()
    text_index: int = readelf_output.find(".text")
    lines: list[str] = readelf_output[text_index:].split("\n")[:2]
    offset: int = int(lines[0].split()[3], 16)
    print(offset)
    size: int = int(lines[1].split()[0], 16)
    print(size)
    with open("scaf", "rb") as f:
        scaf: bytes = f.read()

    text_section: bytes = scaf[offset:offset + size]
    with open(sys.argv[1], "rb") as f:
        famine: bytearray = bytearray(f.read())

    for idx, byte in enumerate(0x01.to_bytes(8, byteorder="little")):
        famine[0x1000 + idx] = byte 

    for idx, byte in enumerate(0x1018.to_bytes(8, byteorder="little")):
        famine[idx + 0x1008] = byte

    for idx, byte in enumerate((0x1018 + size).to_bytes(8, byteorder="little")):
        famine[idx + 0x1010] = byte

    for idx in range(0, size):
        famine[0x1018 + idx] = scaf[offset + idx]

    with open(sys.argv[1], "wb") as f:
        f.write(famine)

    
