-- write settings and queries here and run
-- sqlite3 inf2700_orders.sqlite3 < assignment1_queries.sql
-- to perform the queries
-- for example:

.mode column
.header on

--######### PART 1 #########--

--TASK 2 --
--a
--SELECT C.customerName, contactLastName, contactFirstName
--FROM Customers
--LIMIT 10

--b
-- SELECT  * customerName, contactLastName, contactFirstName
-- FROM Orders --Customers
-- WHERE shippedDate IS NULL --10;

--c
SELECT C.customerName AS Customer, SUM(OD.quantityOrdered) AS Total
FROM Orders O, Customers C, OrderDetails OD
WHERE O.customerNumber = C.customerNumber
AND O.orderNumber = OD.orderNumber
GROUP BY O.customerNumber
ORDER BY Total DESC;


--SELECT P.productName, T.totalQuantityOrdered
--FROM Products P NATURAL JOIN
    --(SELECT productCode, SUM(quantityOrdered) AS totalQuantityOrdered
    --FROM OrderDetails GROUP BY productCode) AS T
--WHERE T.totalQuantityOrdered >= 1000;

-- TASK 3 --
--1)

--SELECT C.country, C.customerName
--FROM Customers C 
--WHERE C.country LIKE '%Norway%'
--ORDER BY customerName


--2)
--Retrieve all classic car products and their scale.

--SELECT C.productName, C.productScale, P.productLine
--FROM Products C, ProductLines P
--WHERE P.productLine == 'Classic Cars';

--3)
--Create a list of incomplete orders (order status is "In process"). The list must contain order-
-- Number, requiredDate, productName, quantityOrdered and quantityInStock.

--SELECT O.orderNumber, O.status, O.requiredDate, P.productName, P.quantityInStock, OD.quantityOrdered
--FROM Orders O
--JOIN OrderDetails OD
--on O.orderNumber = OD.orderNumber
--JOIN Products P
--on OD.productCode = P.productCode
--WHERE O.status = 'In Process'

--4)
--Create a list of customers where the difference between the total price of all ordered products and
--the total amount of all payments exceeds the credit limit. The list must contain the customer
--name, credit limit, total price, total payment and the difference between the two sums.

--SELECT C.customerNumber, C.customerName, C.creditLimit, SUM(P.amount) as TotalPrice, SUM(P.amount)- C.creditLimit AS Diff
--FROM Customers C
--JOIN  Payments P
--on C.customerNumber = P.customerNumber
--GROUP by C.customerNumber
--HAVING Diff >= 0;

--5)
--Create a list of customers who have ordered all products that customer 219 has ordered.

--SELECT C.customerName, C.customerNumber, P.productCode --COUNT(DISTINCT C.customerNumber) AS TT
--FROM Customers C, Products P 
--WHERE C.customerNumber = '219'

--SELECT C.customerNumber, 

--######### PART 2 #########--

