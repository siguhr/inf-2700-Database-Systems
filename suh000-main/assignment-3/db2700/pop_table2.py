import random

POPULATED_TABLE = "populate2.dbcmd"
TABLE_NAME_IN_DB = "person"
N_ROWS = 1000

def insert_rows(file, name, id, seniority, score):
    file.write("insert into %s values (%d, %d, %d);" % (name, id, seniority, score))
    file.write("\n")


def main():
    f = open(POPULATED_TABLE, 'w')
    f.write("create table %s (id int, seniority int, score int)" % (TABLE_NAME_IN_DB))
    f.write("\n\n\n")

    for i in range(N_ROWS):
        seniority = random.randrange(1, 40)
        score = random.randrange(1, 20)
        insert_rows(f, TABLE_NAME_IN_DB, i, seniority, score)

    f.write("\n\n")
    f.write("show database\n")
    f.write("quit")

if __name__ == "__main__":
    main()