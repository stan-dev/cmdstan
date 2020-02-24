sampler_params <- rstan:::get_sampler_params(stanfit, inc_warmup = TRUE)
stepsizes = data.frame(10:n_warmup+50,sapply(sampler_params, function(x) x[, "stepsize__"])[10:n_warmup+50,])
colnames(stepsizes) <- c("warmup_iter", "chain1","chain2","chain3","chain4")
stepsize <- melt(stepsizes ,  id.vars = 'warmup_iter', variable.name = 'dt')
ggplot(stepsize, aes(warmup_iter,value)) + geom_line(aes(colour = dt))

