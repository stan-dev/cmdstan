library(rstan)
options(mc.cores = parallel::detectCores(logical = FALSE))
rstan_options(auto_write = TRUE)

options('width' = 120)

dat <- read.csv('data/canonical/espeland-et-al/caries.csv',
                header = TRUE, comment.char = '#')
N <- dim(dat)[1]
I <- max(dat$item)
J <- max(dat$coder)
ii <- dat$item
jj <- dat$coder
y <- dat$response

model <- stan_model('stan/dawid-skene-binary-logit.stan')
data <- list(N = N, I = I, J = J, ii = ii, jj = jj, y = y)
logit <- function(v) log(v / (1 - v))
logit_pi_init <- logit(mean(y))  # start at data average
logit_alpha_init <- cbind(rep(1, J), rep(-1, J))  # init 88% sens/spec
init_fun <- function(n) list(logit_pi = logit_pi_init,
                             logit_alpha_init = logit_alpha_init)
fit <- sampling(model, data = data, # init = init_fun,
                chains = 4, iter = 800, seed = 12345)
