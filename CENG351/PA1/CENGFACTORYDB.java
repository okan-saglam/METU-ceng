package ceng351.cengfactorydb;

import java.sql.*;
import java.util.ArrayList;

public class CENGFACTORYDB implements ICENGFACTORYDB{
    /**
     * Place your initialization code inside if required.
     *
     * <p>
     * This function will be called before all other operations. If your implementation
     * need initialization , necessary operations should be done inside this function. For
     * example, you can set your connection to the database server inside this function.
     */

    private static Connection connection = null;

    public void initialize() {
        String user = "e2521904"; // TODO: Your userName
        String password = "o&EMW$*MwXEr"; //  TODO: Your password
        String host = "144.122.71.128"; // host name
        String database = "db2521904"; // TODO: Your database name
        int port = 8080; // port

        String url = "jdbc:mysql://" + host + ":" + port + "/" + database;

        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            connection =  DriverManager.getConnection(url, user, password);
        }
        catch (SQLException | ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    /**
     * Should create the necessary tables when called.
     *
     * @return the number of tables that are created successfully.
     */
    public int createTables() {
        int i = 0;
        String query[] = {     """
                                CREATE TABLE IF NOT EXISTS Factory(
                                    factoryId INTEGER,
                                    factoryName TEXT,
                                    factoryType TEXT,
                                    country TEXT,
                                    PRIMARY KEY (factoryId)
                                );""",
                                """
                                CREATE TABLE IF NOT EXISTS Employee(
                                    employeeId INTEGER,
                                    employeeName TEXT,
                                    department TEXT,
                                    salary INTEGER,
                                    PRIMARY KEY (employeeId)
                                );
                                """,
                                """
                                CREATE TABLE IF NOT EXISTS Works_In(
                                    factoryId INTEGER,
                                    employeeId INTEGER,
                                    startDate DATE,
                                    PRIMARY KEY (factoryId, employeeId),
                                    FOREIGN KEY (factoryId) REFERENCES Factory ON DELETE CASCADE ON UPDATE CASCADE,
                                    FOREIGN KEY (employeeId) REFERENCES Employee ON DELETE CASCADE ON UPDATE CASCADE
                                );
                                """,
                                """
                                CREATE TABLE IF NOT EXISTS Product(
                                    productId INTEGER,
                                    productName TEXT,
                                    productType TEXT,
                                    PRIMARY KEY (productId)
                                );
                                """,
                                """
                                CREATE TABLE IF NOT EXISTS Produce(
                                    factoryId INTEGER,
                                    productId INTEGER,
                                    amount INTEGER,
                                    productionCost INTEGER,
                                    PRIMARY KEY (factoryId, productId),
                                    FOREIGN KEY (factoryId) REFERENCES Factory ON DELETE CASCADE ON UPDATE CASCADE,
                                    FOREIGN KEY (productId) REFERENCES Product ON DELETE CASCADE ON UPDATE CASCADE
                                );
                                """,
                                """
                                CREATE TABLE IF NOT EXISTS Shipment(
                                    factoryId INTEGER,
                                    productId INTEGER,
                                    amount INTEGER,
                                    pricePerUnit INTEGER,
                                    PRIMARY KEY (factoryId, productId),
                                    FOREIGN KEY (factoryId) REFERENCES Factory ON DELETE CASCADE ON UPDATE CASCADE,
                                    FOREIGN KEY (productId) REFERENCES Product ON DELETE CASCADE ON UPDATE CASCADE
                                );
                                """
        };
        for(int j = 0 ; j<6 ; j++){
            try{
                java.sql.Statement statement = connection.createStatement();
                statement.executeUpdate(query[i]);
                i++;
                statement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }

    /**
     * Should drop the tables if exists when called.
     *
     * @return the number of tables are dropped successfully.
     */
    public int dropTables() {
        int i=0;
        String query[] = {
            """
                DROP TABLE IF EXISTS Shipment;
            """,
            """
                DROP TABLE IF EXISTS Produce;
            """,
                """
                DROP TABLE IF EXISTS Works_In;
            """,
            """
                DROP TABLE IF EXISTS Product;
            """,
            """
                DROP TABLE IF EXISTS Factory;
            """,
            """
                DROP TABLE IF EXISTS Employee;
            """
        };

        for(int j = 0 ; j < 6 ; j++){
            try{
                Statement statement = connection.createStatement();
                statement.executeUpdate(query[j]);
                i++;
                statement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }

    /**
     * Should insert an array of Factory into the database.
     *
     * @return Number of rows inserted successfully.
     */
    public int insertFactory(Factory[] factories) {
        int i=0;
        String query =  """
                            INSERT INTO Factory(factoryId, factoryName, factoryType, country)
                            VALUES (?, ?, ?, ?);
                        """;

        for(int j=0 ; j<factories.length ; j++){
            try{
                PreparedStatement preparedStatement = connection.prepareStatement(query);
                preparedStatement.setInt(1, factories[j].getFactoryId());
                preparedStatement.setString(2, factories[j].getFactoryName());
                preparedStatement.setString(3, factories[j].getFactoryType());
                preparedStatement.setString(4, factories[j].getCountry());
                preparedStatement.executeUpdate();

                i++;
                preparedStatement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }

    /**
     * Should insert an array of Employee into the database.
     *
     * @return Number of rows inserted successfully.
     */
    public int insertEmployee(Employee[] employees) {
        int i=0;
        String query =  """
                            INSERT INTO Employee(employeeId, employeeName, department, salary)
                            VALUES (?, ?, ?, ?);
                        """;

        for(int j=0 ; j<employees.length ; j++){
            try{
                PreparedStatement preparedStatement = connection.prepareStatement(query);
                preparedStatement.setInt(1, employees[j].getEmployeeId());
                preparedStatement.setString(2, employees[j].getEmployeeName());
                preparedStatement.setString(3, employees[j].getDepartment());
                preparedStatement.setInt(4, employees[j].getSalary());
                preparedStatement.executeUpdate();
                i++;
                preparedStatement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }

    /**
     * Should insert an array of WorksIn into the database.
     *
     * @return Number of rows inserted successfully.
     */
    public int insertWorksIn(WorksIn[] worksIns) {
        int i=0;
        String query =  """
                            INSERT INTO Works_In(factoryId, employeeId, startDate)
                            VALUES (?, ?, ?);
                        """;

        for(int j=0 ; j<worksIns.length ; j++){
            try{
                PreparedStatement preparedStatement = connection.prepareStatement(query);
                preparedStatement.setInt(1, worksIns[j].getFactoryId());
                preparedStatement.setInt(2, worksIns[j].getEmployeeId());
                preparedStatement.setString(3, worksIns[j].getStartDate());
                preparedStatement.executeUpdate();
                i++;
                preparedStatement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }

    /**
     * Should insert an array of Product into the database.
     *
     * @return Number of rows inserted successfully.
     */
    public int insertProduct(Product[] products) {
        int i=0;
        String query =  """
                            INSERT INTO Product(productId, productName, productType)
                            VALUES (?, ?, ?);
                        """;

        for(int j=0 ; j<products.length ; j++){
            try{
                PreparedStatement preparedStatement = connection.prepareStatement(query);
                preparedStatement.setInt(1, products[j].getProductId());
                preparedStatement.setString(2, products[j].getProductName());
                preparedStatement.setString(3, products[j].getProductType());
                preparedStatement.executeUpdate();
                i++;
                preparedStatement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }


    /**
     * Should insert an array of Produce into the database.
     *
     * @return Number of rows inserted successfully.
     */
    public int insertProduce(Produce[] produces) {
        int i=0;
        String query =  """
                            INSERT INTO Produce(factoryId, productId, amount, productionCost)
                            VALUES (?, ?, ?, ?);
                        """;

        for(int j=0 ; j<produces.length ; j++){
            try{
                PreparedStatement preparedStatement = connection.prepareStatement(query);
                preparedStatement.setInt(1, produces[j].getFactoryId());
                preparedStatement.setInt(2, produces[j].getProductId());
                preparedStatement.setInt(3, produces[j].getAmount());
                preparedStatement.setInt(4, produces[j].getProductionCost());
                preparedStatement.executeUpdate();
                i++;
                preparedStatement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }


    /**
     * Should insert an array of Shipment into the database.
     *
     * @return Number of rows inserted successfully.
     */
    public int insertShipment(Shipment[] shipments) {
        int i=0;
        String query =  """
                            INSERT INTO Shipment(factoryId, productId, amount, pricePerUnit)
                            VALUES (?, ?, ?, ?);
                        """;

        for(int j=0 ; j<shipments.length ; j++){
            try{
                PreparedStatement preparedStatement = connection.prepareStatement(query);
                preparedStatement.setInt(1, shipments[j].getFactoryId());
                preparedStatement.setInt(2, shipments[j].getProductId());
                preparedStatement.setInt(3, shipments[j].getAmount());
                preparedStatement.setInt(4, shipments[j].getPricePerUnit());
                preparedStatement.executeUpdate();
                i++;
                preparedStatement.close();
            }
            catch(java.sql.SQLException e){
                e.printStackTrace();
            }
        }
        return i;
    }

    /**
     * Should return all factories that are located in a particular country.
     *
     * @return Factory[]
     */
    public Factory[] getFactoriesForGivenCountry(String country) {
        Factory[] wanted = new Factory[0];
        ArrayList<Factory> list = new ArrayList<Factory>();

        String query =  """
                         SELECT DISTINCT F.factoryId, F.factoryName, F.factoryType, F.country
                         FROM Factory F
                         WHERE F.country = ?
                         ORDER BY F.factoryId ASC;
                        """;

        try{
            PreparedStatement preparedStatement = connection.prepareStatement(query);
            preparedStatement.setString(1, country);
            ResultSet resultSet = preparedStatement.executeQuery();

            while (resultSet.next()){
                int factoryId = resultSet.getInt("factoryId");
                String factoryName = resultSet.getString("factoryName");
                String factoryType = resultSet.getString("factoryType");

                Factory factory = new Factory(factoryId, factoryName, factoryType, country);
                list.add(factory);
            }

            wanted = list.toArray(wanted);

            preparedStatement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }



    /**
     * Should return all factories without any working employees.
     *
     * @return Factory[]
     */
    public Factory[] getFactoriesWithoutAnyEmployee() {
        Factory[] wanted = new Factory[0];
        ArrayList<Factory> list = new ArrayList<Factory>();

        String query =  """
                         SELECT DISTINCT F.factoryId, F.factoryName, F.factoryType, F.country
                         FROM Factory F
                         WHERE F.factoryId NOT IN (
                                                SELECT DISTINCT W.factoryId
                                                FROM Works_In W
                                            )
                         ORDER BY F.factoryId ASC;
                        """;

        try{
            Statement statement = connection.createStatement();
            ResultSet resultSet = statement.executeQuery(query);

            while (resultSet.next()){
                int factoryId = resultSet.getInt("factoryId");
                String factoryName = resultSet.getString("factoryName");
                String factoryType = resultSet.getString("factoryType");
                String country = resultSet.getString("country");

                Factory factory = new Factory(factoryId, factoryName, factoryType, country);
                list.add(factory);
            }

            wanted = list.toArray(wanted);

            statement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }

    /**
     * Should return all factories that produce all products for a particular productType
     *
     * @return Factory[]
     */
    public Factory[] getFactoriesProducingAllForGivenType(String productType) {
        Factory[] wanted = new Factory[0];
        ArrayList<Factory> list = new ArrayList<Factory>();

        String query =  """
                         SELECT DISTINCT F.factoryId, F.factoryName, F.factoryType, F.country
                         FROM Factory F
                         WHERE NOT EXISTS (
                                        SELECT P.productId
                                        FROM Product P
                                        WHERE P.productType = ?
                                        EXCEPT
                                        SELECT PR.productId
                                        FROM Produce PR
                                        WHERE PR.factoryId = F.factoryId
                                    )
                         ORDER BY F.factoryId ASC;
                        """;

        try{
            PreparedStatement preparedStatement = connection.prepareStatement(query);
            preparedStatement.setString(1, productType);
            ResultSet resultSet = preparedStatement.executeQuery();

            while (resultSet.next()){
                int factoryId = resultSet.getInt("factoryId");
                String factoryName = resultSet.getString("factoryName");
                String factoryType = resultSet.getString("factoryType");
                String country = resultSet.getString("country");

                Factory factory = new Factory(factoryId, factoryName, factoryType, country);
                list.add(factory);
            }

            wanted = list.toArray(wanted);

            preparedStatement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }


    /**
     * Should return the products that are produced in a particular factory but
     * don’t have any shipment from that factory.
     *
     * @return Product[]
     */
    public Product[] getProductsProducedNotShipped() {
        // Bir daha bak!!!
        Product[] wanted = new Product[0];
        ArrayList<Product> list = new ArrayList<Product>();

        String query =  """
                            SELECT DISTINCT P.productId, P.productName, P.productType
                            FROM Product P, Produce PR
                            WHERE P.productId = PR.productId AND PR.factoryId NOT IN (
                                                                                    SELECT S.factoryId
                                                                                    FROM Shipment S
                                                                                    WHERE S.productId = P.productId
                                                                                )
                            ORDER BY P.productId ASC;
                        """;

        try{
            Statement statement = connection.createStatement();
            ResultSet resultSet = statement.executeQuery(query);

            while (resultSet.next()){
                int productId = resultSet.getInt("productId");
                String productName = resultSet.getString("productName");
                String productType = resultSet.getString("productType");

                Product product = new Product(productId, productName, productType);
                list.add(product);
            }

            wanted = list.toArray(wanted);

            statement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }


    /**
     * For a given factoryId and department, should return the average salary of
     *     the employees working in that factory and that specific department.
     *
     * @return double
     */
    public double getAverageSalaryForFactoryDepartment(int factoryId, String department) {
        double average=0.0;

        String query =  """
                            SELECT AVG(E.salary)
                            FROM Works_In W, Employee E
                            WHERE E.employeeId = W.employeeId AND W.factoryId = ? AND E.department = ?;
                        """;

        try{
            PreparedStatement preparedStatement = connection.prepareStatement(query);
            preparedStatement.setInt(1, factoryId);
            preparedStatement.setString(2, department);

            ResultSet resultSet = preparedStatement.executeQuery();

            if(resultSet.next()){
                average = resultSet.getDouble(1);
            }

            preparedStatement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return average;
    }


    /**
     * Should return the most profitable products for each factory
     *
     * @return QueryResult.MostValueableProduct[]
     */
    public QueryResult.MostValueableProduct[] getMostValueableProducts() {

        QueryResult.MostValueableProduct[] wanted = new QueryResult.MostValueableProduct[0];
        ArrayList<QueryResult.MostValueableProduct> list = new ArrayList<>();

        String query =  """
                            WITH AllProfits AS (
                                SELECT PR.factoryId, PR.productId, P.productName, P.productType,
                                (COALESCE(S.amount, 0) * COALESCE(S.pricePerUnit, 0) - (PR.amount * PR.productionCost)) AS profit
                                FROM Produce PR
                                LEFT JOIN Shipment S ON S.factoryId = PR.factoryId AND S.productId = PR.productId
                                JOIN Product P ON PR.productId = P.productId
                            )
                            SELECT ALL1.factoryId, ALL1.productId, ALL1.productName, ALL1.productType, ALL1.profit
                            FROM AllProfits ALL1
                            WHERE ALL1.profit = (
                                SELECT MAX(ALL2.profit)
                                FROM AllProfits ALL2
                                WHERE ALL2.factoryId = ALL1.factoryId
                            )
                            ORDER BY profit DESC, factoryId ASC
                        """;

        try{
            Statement statement = connection.createStatement();
            ResultSet resultSet = statement.executeQuery(query);

            while(resultSet.next()){
                int factoryId = resultSet.getInt(1);
                int productId = resultSet.getInt(2);
                String productName = resultSet.getString(3);
                String productType = resultSet.getString(4);
                double profit = resultSet.getDouble(5);

                QueryResult.MostValueableProduct product = new QueryResult.MostValueableProduct(factoryId, productId, productName, productType, profit);
                list.add(product);
            }

            wanted = list.toArray(wanted);

            statement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }

    /**
     * For each product, return the factories that gather the highest profit
     * for that product
     *
     * @return QueryResult.MostValueableProduct[]
     */
    public QueryResult.MostValueableProduct[] getMostValueableProductsOnFactory() {

        QueryResult.MostValueableProduct[] wanted = new QueryResult.MostValueableProduct[0];
        ArrayList<QueryResult.MostValueableProduct> list = new ArrayList<>();

        String query =  """
                            WITH AllProfits AS (
                                SELECT PR.factoryId, PR.productId, P.productName, P.productType,
                                (COALESCE(S.amount, 0) * COALESCE(S.pricePerUnit, 0) - (PR.amount * PR.productionCost)) AS profit
                                FROM Produce PR
                                LEFT JOIN Shipment S ON S.factoryId = PR.factoryId AND S.productId = PR.productId
                                JOIN Product P ON PR.productId = P.productId
                            )
                            SELECT ALL1.factoryId, ALL1.productId, ALL1.productName, ALL1.productType, ALL1.profit
                            FROM AllProfits ALL1
                            WHERE ALL1.profit = (
                                SELECT MAX(ALL2.profit)
                                FROM AllProfits ALL2
                                WHERE ALL2.productId = ALL1.productId
                            )
                            ORDER BY profit DESC, productId ASC
                        """;

        try{
            Statement statement = connection.createStatement();
            ResultSet resultSet = statement.executeQuery(query);

            while(resultSet.next()){
                int factoryId = resultSet.getInt(1);
                int productId = resultSet.getInt(2);
                String productName = resultSet.getString(3);
                String productType = resultSet.getString(4);
                double profit = resultSet.getDouble(5);

                QueryResult.MostValueableProduct product = new QueryResult.MostValueableProduct(factoryId, productId, productName, productType, profit);
                list.add(product);
            }

            wanted = list.toArray(wanted);

            statement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }


    /**
     * For each department, should return all employees that are paid under the
     *     average salary for that department. You consider the employees
     *     that do not work earning ”0”.
     *
     * @return QueryResult.LowSalaryEmployees[]
     */
    public QueryResult.LowSalaryEmployees[] getLowSalaryEmployeesForDepartments() {

        QueryResult.LowSalaryEmployees[] wanted = new QueryResult.LowSalaryEmployees[0];
        ArrayList<QueryResult.LowSalaryEmployees> list = new ArrayList<>();

        String query =  """
                            SELECT DISTINCT E.employeeId, E.employeeName, E.department, E.salary
                            FROM Employee E, Works_In W
                            WHERE E.employeeId = W.employeeId AND E.salary < (
                                SELECT AVG(modifiedEmployee.salary)
                                FROM(
                                    SELECT E4.salary, E4.department
                                    FROM Employee E4, Works_In W4
                                    WHERE W4.employeeId = E4.employeeId
                                    UNION
                                    SELECT E5.salary = 0, E5.department
                                    FROM Employee E5
                                    WHERE E5.employeeId NOT IN (
                                        SELECT W5.employeeId
                                        From Works_In W5
                                    )
                                ) AS modifiedEmployee
                                GROUP BY modifiedEmployee.department
                                HAVING modifiedEmployee.department = E.department
                            )
                            UNION
                            SELECT DISTINCT E3.employeeId, E3.employeeName, E3.department, E3.salary = 0
                            FROM Employee E3
                            WHERE E3.employeeId NOT IN (
                                SELECT W3.employeeId
                                From Works_In W3
                            )
                            ORDER BY employeeId ASC;
                        """;

        try{
            Statement statement = connection.createStatement();
            ResultSet resultSet = statement.executeQuery(query);

            while(resultSet.next()){
                int employeeId = resultSet.getInt(1);
                String employeeName = resultSet.getString(2);
                String department = resultSet.getString(3);
                int salary = resultSet.getInt(4);

                QueryResult.LowSalaryEmployees employee = new QueryResult.LowSalaryEmployees(employeeId, employeeName, department, salary);
                list.add(employee);
            }

            wanted = list.toArray(wanted);

            statement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }


    /**
     * For the products of given productType, increase the productionCost of every unit by a given percentage.
     *
     * @return number of rows affected
     */
    public int increaseCost(String productType, double percentage) {
        int wanted = 0;

        String query =  """
                            UPDATE Produce PR, Product P
                            SET PR.productionCost = (PR.productionCost *  (1+?))
                            WHERE PR.productId = P.productId AND P.productType = ?
                        """;

        try{
            PreparedStatement preparedStatement = connection.prepareStatement(query);
            preparedStatement.setDouble(1, percentage);
            preparedStatement.setString(2, productType);

            wanted = preparedStatement.executeUpdate();

            preparedStatement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }


    /**
     * Deleting all employees that have not worked since the given date.
     *
     * @return number of rows affected
     */
    public int deleteNotWorkingEmployees(String givenDate) {
        int wanted = 0;

        String query =  """
                           DELETE FROM Employee
                           WHERE employeeId NOT IN (
                               SELECT E.employeeId
                               FROM Employee E, Works_In W
                               WHERE W.employeeId = E.employeeId AND W.startDate >= ?
                           );
                        """;

        try{
            PreparedStatement preparedStatement = connection.prepareStatement(query);
            preparedStatement.setString(1, givenDate);

            wanted = preparedStatement.executeUpdate();

            preparedStatement.close();
        }
        catch(java.sql.SQLException e){
            e.printStackTrace();
        }

        return wanted;
    }
}
