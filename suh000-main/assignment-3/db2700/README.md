\
How to run and create table

1. Navigate to the db2700 directory
2. Run "make"
3. Run "python3 pop_table.py" to create populated_1.dbcmd
4. Run "python3 pop_table2.py" to create populated_2.dbcmd
5. Run "make front"
6. Run "./run-front < populate.dbcmd"
7. Run "./run-front < populate2.dbcmd"
8. Run "./run_front"

9. Write a query line. E.g: "select * from workers natural join person where income > 995;"
10. Output is displayed in terminal

Change between nested-loop join and block nested-loop join
11. Enter schema.c and navigate to line 956
12. To run block nested-loop join: uncomment line 956 and comment out line 955
13. To run nested-loop join: uncomment line 955 and and comment out line 956 
-   Run "make front" after the change

Change number of rows in tables
14. Navigate to pop_table.py and pop_table2.py
15. Change the number at N_ROWS to the desired record size. You need to populate the tables again. 


