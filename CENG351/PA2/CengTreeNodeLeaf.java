import java.util.ArrayList;

public class CengTreeNodeLeaf extends CengTreeNode
{
    private ArrayList<CengBook> books;

    // TODO: Any extra attributes

    public CengTreeNodeLeaf(CengTreeNode parent)
    {
        super(parent);

        // TODO: Extra initializations

        this.type = CengNodeType.Leaf;
        this.books = new ArrayList<CengBook>();
    }

    // GUI Methods - Do not modify
    public int bookCount()
    {
        return books.size();
    }
    public Integer bookKeyAtIndex(Integer index)
    {
        if(index >= this.bookCount()) {
            return -1;
        } else {
            CengBook book = this.books.get(index);

            return book.getBookID();
        }
    }

    // Extra Functions

    public void insert_book(CengTree tree, CengBook book){
        int size = this.bookCount();
        int d = this.getOrder();
        int two_d = 2*d;

        if(size+1 > two_d){
            CengBook shiftedBook;
            CengTreeNodeInternal parent;
            CengTreeNodeLeaf newLeaf = new CengTreeNodeLeaf(this.getParent());

            int i = 0;
            while(i < this.bookCount() && this.bookKeyAtIndex(i) < book.getBookID()) i++;
            this.books.add(i, book);

            i=two_d+1;
            while(i > two_d/2) {
                shiftedBook = this.books.get(this.books.size()-1);
                newLeaf.books.add(0, shiftedBook);
                this.books.remove(this.books.size()-1);
                i--;
            }

            if(this.getParent() == null){
                parent = new CengTreeNodeInternal(null);
                this.setParent(parent);
                newLeaf.setParent(parent);
                parent.getAllChildren().add(this);
                parent.getAllChildren().add(newLeaf);
                tree.root = parent;
            }
            else{
                parent = (CengTreeNodeInternal) this.getParent();
                int index = parent.getAllChildren().indexOf(this);
                parent.getAllChildren().add(index+1, newLeaf);
            }

            parent.insert_key(tree, newLeaf.books.get(0).getBookID());
        }
        else{
            int bookId = book.getBookID();
            int i=0;
            while(i < this.bookCount() && this.bookKeyAtIndex(i) < bookId) i++;
            this.books.add(i, book);
        }
    }

    @Override
    public void printTreeNodeRec(CengTreeNode node, int level){

        CengTreeNodeLeaf given = (CengTreeNodeLeaf) node;
        int givenSize = given.bookCount();
        ArrayList<CengBook> givenBooks = given.books;

        String neededTab = "\t".repeat(level);

        System.out.println(neededTab + "<data>");
        for(int i = 0 ; i < givenSize ; i++){
            System.out.print(neededTab + "<record>");
            System.out.print(givenBooks.get(i).fullName());
            System.out.println("</record>");
        }
        System.out.println(neededTab + "</data>");
    }

    @Override
    public void printTreeNode(int level, Integer bookId) {

        ArrayList<CengBook> givenBooks = this.books;

        String neededTab = "\t".repeat(level);

        int i=0;
        while(i < givenBooks.size() && givenBooks.get(i).getBookID() != bookId) i++;
        System.out.print(neededTab + "<record>");
        System.out.print(givenBooks.get(i).fullName());
        System.out.println("</record>");
    }
}
