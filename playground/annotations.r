
# This script reads data from the PantherDB project and takes a pair of functions
# per set of gene trees so that later it estimates the model parameters and predicts
# Them
rm(list = ls())

library(aphylo)
library(ape)

# Reading in the tree data
dat <- read.table(
  "../aphylo_paper/data-raw/experimental_function",
  sep="\t",
  stringsAsFactors = FALSE,
  col.names = c("node_id", "lead_id", "go_id", "annotation")
  )

# Getting the panther book
dat$book_id <- gsub("[:].+$", "", dat$node_id)

# Coding annotations
dat$status <- with(
  dat, 
  ifelse(annotation == "NOT", 0L, ifelse(annotation == "", NA, 1L))
  )

library(dplyr)
counts <- group_by(dat, book_id, go_id) %>%
  summarize(
    nas   = sum(is.na(status)),
    zeros = sum(status[!is.na(status)] == 0),
    ones  = sum(status[!is.na(status)] == 1)
  )

counts <- counts[with(counts, order(zeros, ones, decreasing = TRUE)),]

# This is the set of books that we should use
counts_relevant <- as.data.frame(subset(counts, (zeros != 0) & (ones != 0)))

# Book PTHR23255 and go GO:0050431 is nice
# PTHR10788
# GO:0004805

read_aphylo <- function(family, fun) {
  ans <- read_panther(sprintf("../PANTHER11.1/books/%s/tree.tree", family))
  
  ids <- paste(family, gsub("[:].+", "", ans$tree$tip.label), sep = ":")
  
  A <- subset(dat, (book_id == family) & (node_id %in% ids) & (go_id == fun))
  A$node_id <- ans$tree$tip.label[match(A$node_id, ids)]
  
  # Creating the aphylo object
  try(new_aphylo(
    data.frame(A$node_id, fun= A$status, stringsAsFactors = FALSE,
               check.names = FALSE),
    as_po_tree(ans$tree)
  ))
  
}

L <- Map(
  read_aphylo,
  family = as.list(counts_relevant$book_id),
  fun    = as.list(counts_relevant$go_id)
  )

atree <- L[[2]]
ans_mle  <- aphylo_mle(atree, priors = function(x) dbeta(x, 2, 30))
ans_mcmc <- aphylo_mcmc(ans_mle$par, atree, priors = function(x) dbeta(x, 2, 30),
                        control=list(nbatch=1e5, thin=100, burnin=1e4, nchains=5))

score <- prediction_score(ans_mcmc)
plot(score)
Matrix::image(
  methods::as(cbind(
    predict(ans_mcmc, what = names(which(ans_mcmc$dat$annotations[,1] != 9))),
    ans_mcmc$dat$annotations[names(which(ans_mcmc$dat$annotations[,1] != 9)),]
  ), "dgCMatrix"))

