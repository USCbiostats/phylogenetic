context("Parameter estimation")

rm(list = ls())
set.seed(154)
n <- 500
P <- 1
psi <- c(.01, .01)
mu  <- c(.02, .05)
Pi  <- .2

dat <- sim_annotated_tree(n, P=P, psi = psi, mu = mu, Pi = Pi)
summary(dat)

# Estimation via L-BFGS-B
ans0 <- aphylo_mle(dat, params = rep(.05, 5), priors = function(p) dbeta(p, 2, 20))
ans1 <- aphylo_mcmc(dat = dat, params = c(psi, mu, Pi), priors = function(p) dbeta(p, 2, 20),
                    control=list(nbatch=1e4, thin=50, burnin=1e3, nchains=5))
ans1
plot(ans1$hist)

# Methods ----------------------------------------------------------------------
test_that("Methods", {
  
  # Printing
  expect_output(suppressWarnings(print(ans0)), "ESTIMATION OF")
  
  # Plotting
  expect_silent(plot(ans0))
  expect_silent(plot_LogLike(ans0))
  
  # Extracting coef and others
  expect_equal(coef(ans0), ans0$par)
  expect_equal(vcov(ans0), ans0$varcovar)
  
  
})

# ------------------------------------------------------------------------------
test_that("MCMC", {
  ans1 <- suppressWarnings(
    aphylo_mcmc(params = ans0$par, dat, control = list(nbatch = 1e4, burnin=500, thin=20))
  )
  
  # Checking expectations
  expect_equal(ans0$par, ans1$par, tolerance = 0.2)
  
  # Plotting
  expect_silent(plot(ans1))
  expect_silent(plot_LogLike(ans1))
  
  # Extracting coef and others
  expect_equal(coef(ans1), ans1$par)
  expect_equal(vcov(ans1), ans1$varcovar)
})

test_that("MCMC: in a degenerate case all parameters goes to the prior", {
  
  set.seed(1)
  tree <- sim_tree(10)
  A    <- which(aphylo:::isleaf(tree)) - 1L
  A    <- cbind(A, sample(c(NA), length(A), TRUE))
  dat  <- new_aphylo(A, tree)
  
  ans1 <- suppressWarnings(
    aphylo_mcmc(rep(2/12, 5), dat, priors = function(x) dbeta(x, 2, 10),
                control = list(nbatch = 2e4))
    )
  
  ans2 <- suppressWarnings(
    aphylo_mcmc(rep(2/32, 5), dat, priors = function(x) dbeta(x, 2, 30),
                control = list(nbatch = 2e4))
  )
  
  # Should converge to the prior
  expect_equal(unname(coef(ans1)), rep(2/12, 5), tol=.05)
  expect_equal(unname(coef(ans2)), rep(2/32, 5), tol=.025)
  
})
