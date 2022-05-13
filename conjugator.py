import sys
import json



def read_json_data():
    with open("french_data.json", "r") as f:
        data = json.load(f)

    print(data)


def main():
    read_json_data()



if __name__ == "__main__":
    main()