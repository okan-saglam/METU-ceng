import java.util.ArrayList;

public class CengTreeNodeInternal extends CengTreeNode
{
    private ArrayList<Integer> keys;
    private ArrayList<CengTreeNode> children;

    public CengTreeNodeInternal(CengTreeNode parent)
    {
        super(parent);

        // TODO: Extra initializations, if necessary.
        this.type = CengNodeType.Internal;
        this.keys = new ArrayList<Integer>();
        this.children = new ArrayList<CengTreeNode>();
    }

    // GUI Methods - Do not modify
    public ArrayList<CengTreeNode> getAllChildren()
    {
        return this.children;
    }
    public Integer keyCount()
    {
        return this.keys.size();
    }
    public Integer keyAtIndex(Integer index)
    {
        if(index >= this.keyCount() || index < 0)
        {
            return -1;
        }
        else
        {
            return this.keys.get(index);
        }
    }

    // Extra Function

    public void insert_key(CengTree tree, Integer bookId){
        int order = this.getOrder();
        int size = this.keys.size();
        if(size+1 > 2*order){

            int i = 0;
            while (i<this.keys.size() && this.keys.get(i) < bookId) i++;
            this.keys.add(i, bookId);

            if(this.getParent() != null){
                int key;
                CengTreeNodeInternal parent = (CengTreeNodeInternal) this.getParent();
                CengTreeNodeInternal newInternal = new CengTreeNodeInternal(parent);
                i=2*order;
                while(order < i) {
                    key = this.keys.get(this.keys.size()-1);
                    newInternal.keys.add(0, key);
                    this.keys.remove(this.keys.size()-1);

                    newInternal.getAllChildren().add(0, this.getAllChildren().get(i+1));
                    this.getAllChildren().get(i+1).setParent(newInternal);
                    this.getAllChildren().remove(i+1);

                    i--;
                }
                key = this.keys.get(this.keys.size()-1);
                this.keys.remove(this.keys.size()-1);

                newInternal.getAllChildren().add(0, this.getAllChildren().get(i+1));
                this.getAllChildren().get(i+1).setParent(newInternal);
                this.getAllChildren().remove(i+1);

                int index = parent.getAllChildren().indexOf(this);
                parent.getAllChildren().add(index+1, newInternal);

                parent.insert_key(tree, key);
            }
            else{
                CengTreeNodeInternal root = new CengTreeNodeInternal(null);
                CengTreeNodeInternal sibling = new CengTreeNodeInternal(root);
                this.setParent(root);
                int key;
                i=0;
                while(i<order) {
                    key = this.keys.get(this.keys.size()-1);
                    sibling.keys.add(0, key);
                    this.keys.remove(this.keys.size()-1);
                    i++;
                }
                key = this.keys.get(this.keys.size()-1);
                root.keys.add(0, key);
                this.keys.remove(this.keys.size()-1);
                root.getAllChildren().add(this);
                root.getAllChildren().add(sibling);

                i=2*order+1;
                while(order < i){
                    sibling.getAllChildren().add(0, this.getAllChildren().get(i));
                    this.getAllChildren().get(i).setParent(sibling);
                    this.getAllChildren().remove(i);
                    i--;
                }

                tree.root = root;
            }
        }
        else{
            int i = 0;
            while (i<this.keys.size() && this.keys.get(i) < bookId) i++;
            this.keys.add(i, bookId);
        }
    }

    @Override
    public void printTreeNodeRec(CengTreeNode node, int level){

        CengTreeNodeInternal given = (CengTreeNodeInternal) node;
        int givenSize = given.keyCount();
        ArrayList<Integer> givenKeys = given.keys;
        ArrayList<CengTreeNode> givenChildren = given.getAllChildren();

        String neededTab = "\t".repeat(level);

        System.out.println(neededTab + "<index>");
        for(int i = 0 ; i < givenSize ; i++){
            System.out.println(neededTab + givenKeys.get(i));
        }
        System.out.println(neededTab + "</index>");

        for(int i = 0 ; i < givenChildren.size() ; i++){
            givenChildren.get(i).printTreeNodeRec(givenChildren.get(i), level+1);
        }
    }

    @Override
    public void printTreeNode(int level, Integer bookId){
        ArrayList<Integer> givenKeys = this.keys;
        ArrayList<CengTreeNode> givenChildren = this.getAllChildren();

        String neededTab = "\t".repeat(level);

        int i=0;
        System.out.println(neededTab + "<index>");
        //if(bookId < givenKeys.get(0)) System.out.println(neededTab + givenKeys.get(0));

        while(i < givenKeys.size()){
            System.out.println(neededTab + givenKeys.get(i));
            i++;
        }
        System.out.println(neededTab + "</index>");
    }
}
