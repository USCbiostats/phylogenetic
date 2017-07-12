#include <RcppArmadillo.h>
#include "misc.h"
using namespace Rcpp;

// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]
double prediction_score_rand(
  const arma::mat & A,
  const arma::mat & W,
  double alpha
) {
  
  unsigned int P = A.n_cols;
  unsigned int P2 = (unsigned int) powf(2u, P);
  unsigned int H = A.n_rows;
  
  arma::imat S = states(P);

  // Computing the probabilities of each state
  arma::vec Pa(P2, arma::fill::ones);
  unsigned int s, p;
  for (s = 0; s < P2; s++)
    for (p = 0; p < P; p++) 
      Pa.at(s) *= (S.at(s, p) == 1)? alpha : (1.0-alpha);
    
  
  unsigned int h, u, r, ah, au;
  double score = 0.0, prods;
  
  for (h = 0; h < H; h++)
    
    // Symmetry of W allows us to do it faster, this should also be true for the
    // states data, ah and au. Will work on that later
    
    for (u = 0; u <= h; u++) 
      // Multiplying states
      for (ah=0; ah<P2; ah++)
        
        for (au=0; au<P2; au++) {
          prods = 0.0;
          
          // Product within PxP
          for (p = 0; p < P; p++)
            for (r = 0; r < P; r++)
              prods += powf((A.at(h, p) - S.at(ah, p))*(A.at(u, r) - S.at(au, r)), 2.0);
          
          // Score at the ah, au level
          if (u != h) score += Pa.at(ah)*Pa.at(au)*powf(prods, 0.5)*W.at(h, u)*2;
          else score += Pa.at(ah)*Pa.at(au)*powf(prods, 0.5)*W.at(h, u);
          
        }
    
    
  
  return score;
  
}
