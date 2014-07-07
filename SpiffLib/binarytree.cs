/* File           :     BinaryTree.cs (Beta 2 .Net )
 *                      Binary Tree Implementation.
 * Date           :     7/1/2001
 * Author         :     Rafat Sarosh - rafat.sarosh@usa.net
 * 
 * Reference      :     DataStructure and other Objects Using C++
 *                      by Michael Main, Walter Savitch.
 */ 

using System;
 
namespace SpiffLib {
	class BinaryTree {
		public BinaryTree ( ): this (0) {}
		public BinaryTree (int key, int value ) {
			Key = key;
			Value = value;
			Left = null;
			Right = null;
		}
            
		public BinaryTree Left;
		public BinaryTree Right;
		public int Key;
		public int Value;
 
		/// <summary>
		/// Inserts the Value in Binary Tree.
		/// </summary>
		/// <param name="root">Root of the Binary Tree.</param>
		/// <param name="key">Value to be inserted.</param>
 
		static public void InsertValue ( ref BinaryTree root, int key, int value ) {
			BinaryTree Cursor = root;
                        
 
			/* 
			 * Remain in loop - keep transversing the tree
			 * till find out the proper node or fall of the tree
			 */ 
			while (Cursor != null) {
				if (key <= Cursor.Key ) { 
					/* if i is smaller then the cursor Value
					 * GO LEFT 
					 */
 
					if (Cursor.Left != null)
						Cursor = Cursor.Left ;
					else
						break;
				} 
				else {
					if (Cursor.Right != null)
						Cursor = Cursor.Right;
					else
						break;
				}
                              
			}
 
			/* Reached to the Last node */
 
			if ( key <= Cursor.Key  ) {
				/*  if i is small or equal to node Value, add it to the left */
 				BinaryTree b = new BinaryTree (key, value);
				Cursor.Left = b;
			} 
			else {
				BinaryTree b = new BinaryTree (key, value);
				Cursor.Right = b;
			}
			return;
 
		}//Insert Value
 
                 
		/// <summary>
		/// Prints the Btree in Backward In-Order traversal
		/// </summary>
		/// <param name="root">root node</param>
		/// <param name="depth">should be one 1</param>
		/// <param name="Tab">if true puts tab</param>
		/// 
 
		static public void Print ( BinaryTree root, int depth, bool Tab ) {
			if (root != null) {
				Print (root.Right, depth + 1, Tab );
				int i = 0;
				while ( i++ < depth && Tab ) {
					Console.Write ("   ");
				}
				Console.WriteLine (root.Key );
				Print (root.Left, depth + 1, Tab );
			}
 
		}//print
 
 		/// <summary>
		/// Transverse in the right direction and get the biggest node
		/// </summary>
		/// <param name="node">Node from where to start the search</param>
		/// <returns>
		/// return the BinaryTree Node which is biggest by the virtue of 
		/// binary tree 
		/// </returns>
		/// 
 
		static private BinaryTree GetMaxRight (ref BinaryTree Cursor ) {
			//As soon as the function gets a Left null of a Node
			//it returns the node.
                  
			if (Cursor == null) return null;
			if (Cursor.Right != null) {
				Cursor = Cursor.Right;
				GetMaxRight (ref Cursor);
			}
			return Cursor;
		}
 
		/// <summary>
		/// For the Value, delete the node from the tree.
		/// </summary>
		/// <param name="Cursor">Root of the Tree</param>
		/// <param name="i">Value to be deleted</param>
		/// <returns></returns>
		/// 
 
		static public bool Delete ( ref BinaryTree Cursor, int i ) {
                        
			/* 
			 * Find the node for the Value, we will come out
			 * of this loop only after finding the Value or reaching at
			 * end of tree.
			 * 
			 */
 
			while (Cursor != null) {
				if (Cursor.Key == i ) {
					break; //got the Value- Get out of the loop
				}
 
				/* Is the Value in question more then Node Value
				 * Then go towards Right 
				 */
				if (i > Cursor.Key ) {
					if (Cursor.Right != null)
						Cursor = Cursor.Right ;
					else
						return false; /* Oouch - going back did't find the Value */
				} 
				else { 
                    /* 
					 * Value in Question is smaller 
					 * We are heading towards Left 
					 */
					if (Cursor.Left != null)
						Cursor = Cursor.Left ;
					else
						return false; //going back did't find the Value
				}
			}
      
			/* got the node only if it is NOT null, if it is null
			 * get lost
			 */
 
			if (Cursor == null) return false ;
 
			/* 
			 * Is it a leave 
			 */
			if (Cursor.Left == null && Cursor.Right == null ) {
				/* Simply remove it */
				Cursor.Key  = 0; 
				return true;
			}
                        
			if (Cursor.Left == null ) {
				/* Case 1
				 * node does't have a left child, then simpley promote the right
				 * node to  root.
				 *
    			 * NOTE: Must keep the right node in a temp Node
				 * Cursor.Right = Cursor.Right.Right
				 * Moves the whole Cursor, altogether
				 * 
				 *        pittsburgh                      Seattle
				 *          /    \                          /  \
				 *      null          Seattle   =>          /   \
				 *                      / \              Left    Right
				 *                  Left   Right
				 */
                               
				BinaryTree TempNode = Cursor.Right ;
				Cursor.Key   = TempNode.Key ; 
				Cursor.Right.Key   = 0 ;           //Mark it for deletion
				Cursor.Right = TempNode.Right ;
				if (TempNode.Left != null)
					Cursor.Left  = TempNode.Left ;
				else
					Cursor.Left = null;
				return true;
			} 
			else {
				/* case 2
				 * Node *DOES* have a LEFT child, find the MAX RIGHT child 
				 * of LEFT NODE and then  make that Node as root
				 *
				 * To Delete A 
				 * Get the biggest Node from the Right hand side
				 * 
				 *       A                     R2
				 * 	   /    \                /    \
				 * 	  /      \              /      \
				 *    L1      R1    =>      L1      R1
				 *   / \      / \          /       /  \
				 *  L2 R2   RL1 RR2      L2      RL1  RR2
				 */
                              
				//Got the Node in the Question, keep it in Temp
				BinaryTree _Node = Cursor;   
				//Get on the Left Node
				Cursor = Cursor.Left ; 
				//Find out Max Right
				BinaryTree BigRNode = GetMaxRight ( ref Cursor ); 
				_Node.Key  = BigRNode.Key ;
				BigRNode.Key = 0; //Mark it for deletion
				return true;
			}
		}//Delete
 
		static public void MakeNull (ref BinaryTree root) {
 			if (root != null ) {
				if ( root.Key == 0 ) {
					root = null ;
					return;
				}
 
				MakeNull(ref root.Left  );
				MakeNull(ref root.Right );
			}
 
		} 
	}//BinaryTree
}//nameSpace
