import sys
import json



def read_json_data():
    with open("french_data.json", "r") as f:
        data = json.load(f)

    print(data["verbs"]["\u00EAtre"]["indicative"]["present"][1])


def main():
    read_json_data()



if __name__ == "__main__":
    main()