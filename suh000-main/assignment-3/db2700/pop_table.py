import random

POPULATED_TABLE = "populate.dbcmd"
TABLE_NAME_IN_DB = "workers"
N_ROWS = 1000

def insert_rows(file, name, id, income, department):
    file.write("insert into %s values (%d, %d, %d);" % (name, id, income, department))
    file.write("\n")


def main():
    f = open(POPULATED_TABLE, 'w')
    f.write("create table %s (id int, income int, department int)" % (TABLE_NAME_IN_DB))
    f.write("\n\n\n")

    for i in range(N_ROWS):
        income = random.randrange(350, 999)
        department = random.randrange(1,27)
        insert_rows(f, TABLE_NAME_IN_DB, i, income, department)

    f.write("\n\n")
    f.write("show database\n")
    f.write("quit")

if __name__ == "__main__":
    main()