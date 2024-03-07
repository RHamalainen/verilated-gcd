import math
from pathlib import Path

if __name__ == "__main__":
    output_path = Path("testcases.txt")
    with output_path.open("w") as stream:
        value_min = 1
        value_max = 15
        for i in range(value_min, value_max + 1):
            for j in range(value_min, value_max + 1):
                gcd = math.gcd(i, j)
                print(f"{i} {j} {gcd}", file=stream)
