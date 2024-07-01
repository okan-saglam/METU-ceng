import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Scanner;

public class CengTreeParser
{
    public static ArrayList<CengBook> parseBooksFromFile(String filename)
    {
        ArrayList<CengBook> bookList = new ArrayList<CengBook>();

        // You need to parse the input file in order to use GUI tables.
        // TODO: Parse the input file, and convert them into CengBooks

        try(BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String Line;
            CengBook book;
            while((Line = br.readLine()) != null){
                String[] attributes = Line.split("\\|");
                Integer bookID = Integer.parseInt(attributes[0].trim());
                String bookTitle = attributes[1].trim();
                String author = attributes[2].trim();
                String genre = attributes[3].trim();
                book = new CengBook(bookID, bookTitle, author, genre);
                bookList.add(book);
            }
        }
        catch (IOException e){
            e.printStackTrace();
        }

        return bookList;
    }

    public static void startParsingCommandLine() throws IOException
    {
        // TODO: Start listening and parsing command line -System.in-.
        // There are 4 commands:
        // 1) quit : End the app, gracefully. Print nothing, call nothing, just break off your command line loop.
        // 2) add : Parse and create the book, and call CengBookRunner.addBook(newlyCreatedBook).
        // 3) search : Parse the bookID, and call CengBookRunner.searchBook(bookID).
        // 4) print : Print the whole tree, call CengBookRunner.printTree().

        // Commands (quit, add, search, print) are case-insensitive.

        Scanner scanner = new Scanner(System.in);
        while(true){
            String input = scanner.nextLine();
            String[] parts = input.split("\\|");
            switch (parts[0].toLowerCase()) {
                case "quit":
                    return;
                case "add":
                    Integer bookID = Integer.parseInt(parts[1]);
                    String bookTitle = parts[2];
                    String author = parts[3];
                    String genre = parts[4];
                    CengBook newBook = new CengBook(bookID, bookTitle, author, genre);
                    CengBookRunner.addBook(newBook);
                    break;
                case "search":
                    Integer bookId = Integer.parseInt(parts[1]);
                    CengBookRunner.searchBook(bookId);
                    break;
                case "print":
                    CengBookRunner.printTree();
                    break;
                default:
                    break;
            }
        }
    }
}
