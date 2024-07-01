import java.util.ArrayList;

public class CengTree
{
    public CengTreeNode root;
    // Any extra attributes...

    public CengTree(Integer order)
    {
        CengTreeNode.order = order;
        // TODO: Initialize the class
        this.root = new CengTreeNodeLeaf(null);
    }

    public void addBook(CengBook book)
    {
        // TODO: Insert Book to Tree
        CengTreeNodeLeaf correct = childrenToBeAdded(this.root, book);
        correct.insert_book(this, book);
    }

    public ArrayList<CengTreeNode> searchBook(Integer bookID)
    {
        // TODO: Search within whole Tree, return visited nodes.
        // Return null if not found.
        ArrayList<CengTreeNode> wanted = new ArrayList<CengTreeNode>();

        boolean result = find(this.root, bookID);
        if(!result){
            System.out.printf("Could not find %d.\n", bookID);
        }
        else{
            searchHelper(this.root, bookID, -1, wanted);
        }

        return wanted;
    }

    public void printTree()
    {
        // TODO: Print the whole tree to console

        CengTreeNode Root = this.root;
        Root.printTreeNodeRec(Root, 0);
    }

    // Any extra functions...
    public CengTreeNodeLeaf childrenToBeAdded(CengTreeNode treeRoot, CengBook book){
        if(treeRoot instanceof CengTreeNodeLeaf){
            return (CengTreeNodeLeaf) treeRoot;
        }
        else{
            int i=0;
            CengTreeNodeInternal internalNode = (CengTreeNodeInternal) treeRoot;
            while(i < internalNode.keyCount() && internalNode.keyAtIndex(i) < book.getBookID()) i++;
            return childrenToBeAdded(internalNode.getAllChildren().get(i), book);
        }
    }

    public boolean isElement(CengTreeNodeLeaf node, Integer bookId){
        int nodeSize = node.bookCount();
        for(int i = 0 ; i < nodeSize ; i++){
            if(node.bookKeyAtIndex(i) == bookId) return true;
        }
        return false;
    }

    public void searchHelper(CengTreeNode treeRoot, Integer bookId, int level, ArrayList<CengTreeNode> wanted){
        if(treeRoot instanceof CengTreeNodeLeaf){
            wanted.add(treeRoot);
            level++;
            treeRoot.printTreeNode(level, bookId);
        }
        else{
            int i=0;
            level++;
            CengTreeNodeInternal internalNode = (CengTreeNodeInternal) treeRoot;
            wanted.add(internalNode);
            internalNode.printTreeNode(level, bookId);
            while(i < internalNode.keyCount() && internalNode.keyAtIndex(i) <= bookId) i++;
            searchHelper(internalNode.getAllChildren().get(i), bookId, level, wanted);
        }
    }

    public boolean find(CengTreeNode treeNode, Integer bookId){
        if(treeNode instanceof CengTreeNodeLeaf){
            return isElement((CengTreeNodeLeaf) treeNode, bookId);
        }
        else{
            int i=0;
            CengTreeNodeInternal internalNode = (CengTreeNodeInternal) treeNode;
            while(i < internalNode.keyCount() && internalNode.keyAtIndex(i) <= bookId) i++;
            return find(internalNode.getAllChildren().get(i), bookId);
        }
    }
}
