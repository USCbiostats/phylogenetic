#' Removing leafs and nodes from a tree
#' 
#' This function takes one or more nodes/leafs from a given tree and removes them
#' making sure that the position indexes are updated, hence preserving the 
#' `po_tree()` structure.
#' 
#' @param x An object of class `po_tree` or `aphylo`.
#' @param ids Either a vector or a scalar indicating which nodes/leafs to remove.
#' If integer, then its values should be within (0, n-1]. Otherwise, if character
#' it can be the nodes/leafs labels.
#' @param ... Ignored
#' 
#' @return A prunned version of the tree.
#' @family Data management functions
#' @details 
#' 
#' From now we will denote node(s) as either internal node(s) or leaf(s). Given
#' that we want to remove `ids`, the algorithm goes as follows:
#' \enumerate{
#' \item Identifies which nodes are been asked to be removed and
#' checks whether these actually exits.
#' 
#' \item Computes the topological shortest path matrix between pairs of
#' nodes, and if the nodes to be removed are parents of nodes not included
#' in the list, these will be added (the whole branch is removed).
#' 
#' \item Considering which nodes are been removed, a new set of positions
#' is computed so that it follows `po_tree()` convention.
#' 
#' \item The edgelist is updated, as well the labels.
#' 
#' \item The set of offspring is recalculated.
#' }
#' 
#' @examples 
#' 
#' # A simple example of how prune works-------------------------------------------
#' # Simulating a nice tree
#' set.seed(1213)
#' x <- sim_tree(4)
#' 
#' # Setting up the plot envir
#' oldpar <- par(no.readonly = TRUE)
#' par(mfrow = c(3,2), mai=rep(.5,4), cex=1, xpd=NA, omi=c(0,0,1,0))
#' 
#' # Plotting 
#' plot(x, main = "Full tree", show.node.label = TRUE)
#' plot(prune(x, c(2,6)), main="removing (2,6)", show.node.label = TRUE)
#' plot(prune(x, 6), main="removing 6", show.node.label = TRUE)
#' plot(prune(x, 4), main="removing 4", show.node.label = TRUE)
#' plot(prune(x, 3), main="removing 3", show.node.label = TRUE)
#' plot(prune(x, c(4,6,3)), main="removing (4,6,3)", show.node.label = TRUE)
#' 
#' # Adding a title
#' par(mai=rep(1,4), omi=rep(0,4), mfrow=c(1,1), new=FALSE)
#' title("Prunning trees with -prune-")
#' par(oldpar)
#' 
#' # Removing the leafs --------------------------------------------------------
#' 
#' set.seed(1)
#' x <- sim_tree(25)
#' oldpar <- par(no.readonly=TRUE)
#' par(mfrow=c(2,2))
#' plot(x)
#' 
#' plot(prune(x, "leafs"))
#' plot(prune(prune(x, "leafs"), "leafs"))
#' plot(prune(prune(prune(x, "leafs"), "leafs"), "leafs"))
#' par(oldpar)
#' 
#' @name prune
NULL

#' @export
#' @rdname prune
prune <- function(x, ids, ...) .Defunct("tbd", msg ="Working on it")


