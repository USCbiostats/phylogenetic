#' Simulation of Annotated Phylogenetic Trees
#' 
#' @param n Integer scalar. Number of leafs. If not specified, then 
#' @param tree An object of class \code{\link[=sim_tree]{po_tree}}.
#' @param P Integer scalar. Number of functions to generate.
#' @template parameters
#' @templateVar psi 1
#' @templateVar mu 1
#' @templateVar Pi 1
#' @return An object of class \code{\link[=new_aphylo]{aphylo}}
#' @family Simulation Functions 
#' @export
#' @examples 
#' # A simple example ----------------------------------------------------------
#' 
#' set.seed(1231)
#' ans <- sim_annotated_tree(n=500)
#' 
sim_annotated_tree <- function(
  n    = NULL,
  tree = NULL,
  P    = 1,
  psi  = c(.05, .05),
  mu   = c(.1,.05),
  Pi   = 1
  ) {
  
  pars <- unlist(c(psi, mu, Pi), recursive = TRUE)
  names(pars) <- c("psi0", "psi1", "mu0", "mu1", "Pi")
  
  # Step 1: Simulate a tree
  if (!length(tree)) {
    
    # Checking if there's n
    if (!length(n))
      stop("When -tree- is not specified, -n- must be specified.")
    
    ntree <- sim_tree(n)  
    tree  <- ntree
    O     <- attr(ntree, "offspring")
    
  } else {
    if (!inherits(tree, "po_tree"))
      stop("-tree- must be an object of class -po_tree- (see -sim_tree-).")
    
    O  <- list_offspring(tree)
  }
  
  # Step 2: Simulate the annotations
  ans <- sim_fun_on_tree(
    offspring  = O,
    psi        = c(pars["psi0"], pars["psi1"]),
    mu         = c(pars["mu0"], pars["mu1"]),
    Pi         = pars["Pi"],
    P          = P
  )
  
  rownames(ans) <- unname(attr(tree, "labels"))
  
  # Creating the aphylo object
  as_aphylo(
    annotations = ans,
    offspring   = O,
    edges       = tree
  )
  
}


#' @rdname sim_tree
#' @export
#' @param x An object of class \code{sim_tree}
#' @param ... Ignored.
print.aphylo_sim_tree <- function(x, ...) {
  nelems <- length(attr(x$edges, "labels"))
  
  cat("\nA RANDOM PHYLOGENETIC TREE (list with 3 elements):\n\n(1) po_tree ($edges):")
  print(x$edges)
  
  cat("(2) List of offspring ($offspring).\n")

  
  invisible(x)
}
