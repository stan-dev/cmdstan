cc.summary <- function(stanfit) {

(n_warmup = stanfit@sim$warmup)
n_iter = stanfit@sim$iter-n_warmup
sampler_params <- rstan:::get_sampler_params(stanfit, inc_warmup = TRUE)
leapfrogs = sapply(sampler_params, function(x) x[, "n_leapfrog__"])
(sum_warmup_leapfrogs = sum(leapfrogs[1:n_warmup,]))
(sum_leapfrogs = sum(leapfrogs[n_warmup+(1:n_iter),]))
(mean_warmup_leapfrogs = sum_warmup_leapfrogs/n_warmup)
(mean_leapfrogs = sum_leapfrogs/n_iter)
mon = rstan::monitor(as.array(stanfit), warmup=0, print=FALSE)
(maxrhat = max(mon[,'Rhat']))
bulk_ess_per_iter = mon[,'Bulk_ESS']/n_iter
tail_ess_per_iter = mon[,'Tail_ESS']/n_iter
bulk_ess_per_leapfrog = mon[,'Bulk_ESS']/sum_leapfrogs
tail_ess_per_leapfrog = mon[,'Tail_ESS']/sum_leapfrogs
min(bulk_ess_per_iter)
min(tail_ess_per_iter)
min(bulk_ess_per_leapfrog)
min(tail_ess_per_leapfrog)
(stepsizes = sapply(sampler_params, function(x) x[, "stepsize__"])[n_iter,])

## classic warmup
(n_warmup = stanfit@sim$warmup)
n_iter = stanfit@sim$iter-n_warmup
sampler_params <- rstan:::get_sampler_params(stanfit, inc_warmup = TRUE)
leapfrogs = sapply(sampler_params, function(x) x[, "n_leapfrog__"])
(sum_warmup_leapfrogs = sum(leapfrogs[1:n_warmup,]))
(sum_leapfrogs = sum(leapfrogs[n_warmup+(1:n_iter),]))
(mean_warmup_leapfrogs = sum_warmup_leapfrogs/n_warmup)
(mean_leapfrogs = sum_leapfrogs/n_iter)
mon = rstan::monitor(as.array(stanfit), warmup=0, print=FALSE)
(maxrhat = max(mon[,'Rhat']))
bulk_ess_per_iter = mon[,'Bulk_ESS']/n_iter
tail_ess_per_iter = mon[,'Tail_ESS']/n_iter
bulk_ess_per_leapfrog = mon[,'Bulk_ESS']/sum_leapfrogs
tail_ess_per_leapfrog = mon[,'Tail_ESS']/sum_leapfrogs
min(bulk_ess_per_iter)
min(tail_ess_per_iter)
min(bulk_ess_per_leapfrog)
min(tail_ess_per_leapfrog)
(stepsizes = sapply(sampler_params, function(x) x[, "stepsize__"])[n_iter,])

}
