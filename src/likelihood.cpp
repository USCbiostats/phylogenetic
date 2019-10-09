#include <Rcpp.h>
#include "pruner.hpp"
#include "TreeData.hpp" // TreeData definition
using namespace Rcpp;

// #define DEBUG_LIKELIHOOD

void likelihood(
    pruner::sptr_treedata D,
    pruner::TreeIterator & n
) {
  
#ifdef DEBUG_LIKELIHOOD
  printf("Entering likelihood at node %d with pruneseq:\n", *n);
  pruner::v_uint xx = n.tree->get_postorder();
  for (auto iter = xx.begin(); iter != xx.end(); ++iter)
    printf("%i, ", *iter);
  printf("\n");
#endif
  
  if (n.is_tip()) {
    
    // Iterating through the states
    pruner::uint s, p;
    for (s = 0u; s < D->states.size(); ++s) {
      
      // Throught the functions
      D->Pr[*n][s] = 1.0; // Initializing
      for (p = 0u; p < D->nfuns; ++p) {
        
        // ETA PARAMETER
        if (D->A[*n][p] == 9u && (D->eta[0u] >= 0.0)) {
        
          D->Pr[*n][s] *=
            (1.0 - D->eta[0u]) * D->PSI[D->states[s][p]][0u] +
            (1.0 - D->eta[1u]) * D->PSI[D->states[s][p]][1u]
            ;
          
        } else {
          
          // Unnanotated leafs should be skipped in this situation. This mostly
          // happens if we are not using the eta parameter and during cv. At
          // that time some annotations are dropped without modifying the 
          // pruning sequence.
          if (D->A[*n][p] == 9u)
            continue;
        
          if (D->eta[0u] >= 0.0) {
            
            D->Pr[*n][s] *= D->PSI[D->states[s][p]][D->A[*n][p]]*
              D->eta[D->A[*n][p]];
            
          } else {
            
            D->Pr[*n][s] *= D->PSI[D->states[s][p]][D->A[*n][p]];
            
          }
          
        }
          
      }
      
    }
    
  } else {
    
    D->MU[0] = &(D->MU_d);
    D->MU[1] = &(D->MU_s);
    
    std::vector< unsigned int >::const_iterator o_n;
    pruner::uint s_n, p_n, s;
    double offspring_ll, s_n_sum;
    
    // Looping through states
    for (s = 0u; s < D->nstates; ++s) {
      
      // Now through offspring
      D->Pr[*n][s] = 1.0;
      for (o_n = n.begin_off(); o_n != n.end_off(); ++o_n) {
        
        // Offspring state integration
        offspring_ll = 0.0;
        for (s_n = 0u; s_n < D->nstates; ++s_n) {
          
          s_n_sum = 1.0;
          for (p_n = 0u; p_n < D->nfuns; ++p_n)
            // s_n_sum *= (D->MU[D->types[*n]]).at(D->states[s][p_n]).at(D->states[s_n][p_n]);
            s_n_sum *= ((D->types[*n] == 0u)?
              D->MU_d[D->states[s][p_n]][D->states[s_n][p_n]] :
              D->MU_s[D->states[s][p_n]][D->states[s_n][p_n]]) / n.n_offspring();
          
          // Multiplying by off's probability
          offspring_ll += (s_n_sum) * D->Pr[*o_n][s_n];
          
        }
        
        // Getting the joint conditional.
        D->Pr[*n][s] *= offspring_ll;
        
      }
      
    }
    
    // Computing the joint likelihood
    if (*n == n.back()) {
      D->ll = 0.0;
      for (s = 0; s < D->nstates; ++s) 
        D->ll += D->Pi[s] * D->Pr[*n][s];
      D->ll = log(D->ll);
    }
    
  }
  
  
  return;
  
}

// Tree constructor ------------------------------------------------------------

// [[Rcpp::export(name = "new_aphylo_pruner.", rng = false)]]
SEXP new_aphylo_pruner(
    const std::vector< std::vector< unsigned int > > & edgelist,
    const std::vector< std::vector< unsigned int > > & A,
    const std::vector< unsigned int >  & types,
    unsigned int nannotated
) {
  
  // Initializing the tree
  pruner::uint res;
  Rcpp::XPtr< pruner::Tree > xptr(new pruner::Tree(edgelist[0], edgelist[1], res), true);
  
  if (res != 0u)
    stop(
      "An error of code %d happened while creating the pruner::Tree object.",
      res
    );
  
  xptr->args = std::make_shared< pruner::TreeData >(A, types, nannotated);
  xptr->fun  = likelihood;
  
  // Figuring out the corrected pseq; ------------------------------------------
  
  // This flags which to include
  std::vector< bool > has_ann(A.size(), false);
  
  // This is a pointer to the set of offsprings. This is how we check which
  // is leaf or not
  const pruner::vv_uint * offspring = xptr->get_offspring_ptr();
  
  // This is the current POSTORDER sequence. We save it just in case
  pruner::v_uint cur_pseq = xptr->get_postorder();
  pruner::v_uint new_pseq;
  
  // We start iterating through the annotations
  for (auto i = cur_pseq.begin(); i != cur_pseq.end(); ++i) {
    
    // First check if it is leaf or not
    if ((offspring->at(*i).size()) == 0u) {
      
      // Checking annotations
      int n9s = 0;
      for (unsigned int j = 0u; j < A[0].size(); ++j) 
        if (A[*i][j] == 9u) {
          ++n9s;
          break;
        }
        
      // At least has a single annotation!
      if (n9s < A[0u].size()) {
        has_ann[*i] = true;
        new_pseq.push_back(*i);
      }
      
    } else { // The case for interior nodes
      
      // We need to iterate through its offsprings
      for (auto off = (offspring->at(*i)).begin(); off != (offspring->at(*i)).end(); ++off) 
        // Any of its offspring has an annotation?
        if (has_ann[*off]) {
          has_ann[*i] = true;
          new_pseq.push_back(*i);
          break;
        }
      
    }
    
  }
  
  // Resetting the pseq, only if it has nodes on it!
  if (new_pseq.size() != 0u) {
    res = xptr->set_postorder(new_pseq);
    if (res != 0u)
      stop("While resetting the POSTORDER.");
  }

  xptr.attr("class") = "aphylo_pruner";
  
  return xptr;
}

// Methods ---------------------------------------------------------------------

// [[Rcpp::export(name = ".LogLike_pruner", rng = false)]]
List LogLike_pruner(
    SEXP tree_ptr,
    const std::vector< double > & mu_d,
    const std::vector< double > & mu_s,
    const std::vector< double > & psi,
    const std::vector< double > & eta,
    const double & Pi,
    bool verb = true,
    bool check_dims = false
) {
  Rcpp::XPtr< pruner::Tree > p(tree_ptr);
  
  // Setting the parameters
  p->args->set_mu_d(mu_d);
  p->args->set_mu_s(mu_s);
  p->args->set_psi(psi);
  
  p->args->set_eta(eta);
  
  // In the case of Pi, if it is negative, then it means that we are using
  // the stationary value of the transition probabilities.
  if (Pi < 0.0) {
    p->args->set_pi(
        (1 - p->args->prop_type_d)* mu_s[0]/(mu_s[0] + mu_s[1]) +
          p->args->prop_type_d * mu_d[0]/(mu_d[0] + mu_d[1])
    );
  } else 
    p->args->set_pi(Pi);
  
  p->prune_postorder();
  
  if (verb) {
    NumericMatrix Pr(p->args->n, p->args->nstates);
    for (unsigned int i = 0u; i < p->args->n; ++i)
      for (unsigned int j = 0u; j < p->args->nstates; ++j)
        Pr(i, j) = p->args->Pr[i][j];
    
    return List::create(
      _["Pr"] = List::create(Pr),
      _["ll"] = wrap(p->args->ll)
      );
  } else
    return List::create(_["ll"] = wrap(p->args->ll));
}

// [[Rcpp::export(rng = false)]]
std::vector< std::vector< unsigned int > > Tree_get_offspring(const SEXP & tree_ptr) {
  
  Rcpp::XPtr< pruner::Tree > p(tree_ptr);
  return p->get_offspring();
  
}

// [[Rcpp::export(rng = false)]]
std::vector< std::vector< unsigned int > > Tree_get_parents(const SEXP & tree_ptr) {
  
  Rcpp::XPtr< pruner::Tree > p(tree_ptr);
  return p->get_parents();
  
}


// [[Rcpp::export(name=".Nnode_aphylo_pruner", rng = false)]]
unsigned int Tree_Nnode(const SEXP & tree_ptr, bool internal_only = true) {
  
  Rcpp::XPtr< pruner::Tree > p(tree_ptr);
  
  unsigned int count = p->n_nodes();
  
  if (internal_only)
    count -= p->n_tips();
  
  return count;
}

// [[Rcpp::export]]
std::vector< unsigned int > Tree_get_dist_tip2root(const SEXP & tree_ptr) {
  
  Rcpp::XPtr< pruner::Tree > p(tree_ptr);
  
  return p->get_dist_tip2root();
  
}

// [[Rcpp::export]]
std::vector< unsigned int > Tree_get_tips(const SEXP & tree_ptr) {
  
  Rcpp::XPtr< pruner::Tree > p(tree_ptr);
  
  return p->get_tips();
  
}

// [[Rcpp::export]]
std::vector< unsigned int > Tree_get_postorder(const SEXP & tree_ptr) {
  
  Rcpp::XPtr< pruner::Tree > p(tree_ptr);
  
  return p->get_postorder();
  
}

//' @export
//' @rdname ape-methods
// [[Rcpp::export(name="Ntip.aphylo_pruner", rng = false)]]
unsigned int Tree_Ntip(const SEXP & phy) {
  
  Rcpp::XPtr< pruner::Tree > p(phy);
  
  return p->n_tips();
}


//' @rdname ape-methods
//' @export
// [[Rcpp::export(name="Nannotated.aphylo_pruner", rng = false)]]
unsigned int Tree_Nannotated(const SEXP & phy) {
  
  Rcpp::XPtr< pruner::Tree > p(phy);
  
  return p->args->nannotated;
}

//' @rdname ape-methods
//' @export
// [[Rcpp::export(name="Nann.aphylo_pruner", rng = false)]]
unsigned int Tree_Nann(const SEXP & phy) {
  
  Rcpp::XPtr< pruner::Tree > p(phy);
  
  return p->args->nfuns;
}

// [[Rcpp::export]]
std::vector< double > root_node_pr(
  double Pi,
  const std::vector< std::vector<unsigned int> > & S
) {
  
  std::vector< double > pi_probs(S.size());
  root_node_pr(pi_probs, Pi, S);
  
  return pi_probs;
  
}

// [[Rcpp::export]]
unsigned int Tree_set_ann(const SEXP & phy, unsigned int i, unsigned int j, unsigned int val) {
  
  Rcpp::XPtr< pruner::Tree > p(phy);
  
  p->args->set_ann(i, j, val);
  
  return 0u;
  
}

// [[Rcpp::export]]
std::vector< std::vector< unsigned int > > Tree_get_ann(const SEXP & phy) {
  
  Rcpp::XPtr< pruner::Tree > p(phy);
  return p->args->A;
  
}


/***R
set.seed(1)
dat <- aphylo::raphylo(50)

A <- rbind(dat$tip.annotation, dat$node.annotation)[,1]

mu  <- c(.1, .05)
psi <- c(.2, .07)
eta <- c(.8, .9)
Pi  <- .5

tree_ptr <- aphylo:::new_Tree(
  edgelist = with(dat$tree, list(edge[,1] - 1, edge[,2] - 1)),
  A        = as.list(A),
  types    = A
)

aphylo_ll <- aphylo::LogLike

microbenchmark::microbenchmark(
  new = aphylo:::.LogLike2(tree_ptr, mu = mu, psi = psi, eta = eta, pi = Pi),
  old = aphylo::LogLike(dat, psi = psi, mu = mu, Pi = Pi, eta = eta)$ll,
  times = 100, unit = "relative"
)


*/