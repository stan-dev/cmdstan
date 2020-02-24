## Aki's summary script
perf.cc <- function(stanfit) {
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

    res <- data.frame(run = c(sum_warmup_leapfrogs, sum_leapfrogs,
                                   mean_warmup_leapfrogs, mean_leapfrogs,
                                   min(bulk_ess_per_iter), min(tail_ess_per_iter), min(bulk_ess_per_leapfrog), min(tail_ess_per_leapfrog)))
    row.names(res) <- c("warmup.leapfrogs", "leapfrogs",
                        "mean.warmup.leapfrogs", "mean.leapfrogs",
                        "min(bulk_ess/iter)", "min(tail_ess/iter)", "min(bulk_ess/leapfrog)", "min(tail_ess/leapfrog)")
    return(res)
}

## run both cross-chain & regular warmup version and output summary
run.cc.metric <- function(modelpath, model, metric, seed, adapt.arg="") {
    model.file = paste(model,"stan", sep="")
    data.file = paste(model,".data.R", sep="")
    system(paste("make -j4 ", file.path(modelpath, model, model), sep=""))
    cmdwd <- getwd();

    setwd(file.path(modelpath, model))

    system(paste("mpiexec -n 4 -l -bind-to core ./", model, " sample save_warmup=1 algorithm=hmc metric=", metric, " adapt ", adapt.arg, " data file=", data.file, " random seed=", seed, " ",sep=""))
    fit.mpi <- rstan::read_stan_csv(dir(pattern="mpi.[0-3].output.csv", full.name=TRUE))
    system(paste("for i in {0..3}; do ./", model, " sample save_warmup=1 algorithm=hmc metric=", metric, " adapt ", adapt.arg, " data file=", data.file, " random seed=", seed, " output file=$i.output.csv id=$i;done", " ",sep=""))
    fit.seq <- rstan::read_stan_csv(dir(pattern="^[0-3].output.csv", full.name=TRUE))
    summary <- data.frame(perf.cc(fit.mpi), perf.cc(fit.seq))
    colnames(summary) <- c(paste0("MPI.",metric), paste0("regular.",metric))

    setwd(cmdwd)
    return(list(mpi=fit.mpi, seq=fit.seq, summary=summary))
}

## run cross-chain & regular warmup for both diag & dense metric
run.cc <- function(modelpath, model, adapt.arg="") {
    seed = sample(999999:9999999, 1)

    fits.diag  <- run.cc.metric(modelpath, model, "diag_e",  seed, adapt.arg)
    fits.dense <- run.cc.metric(modelpath, model, "dense_e", seed, adapt.arg)

    library("gridExtra")
    library("bayesplot")
    library("dplyr")
    ## pdf(paste0(model, "_cross_chain_summary.pdf"), paper="a4")
    ## grid.table(format(cbind(fits.diag[["summary"]], fits.dense[["summary"]]), digits=3))
    ## lp <- log_posterior(fits.diag[["mpi"]])
    ## np <- nuts_params(fits.diag[["mpi"]])
    ## p <- grid.arrange(mcmc_nuts_divergence(np, lp),top="mcmc_nuts_divergence: cross-chain + diag_e")
    ## print(p)
    ## lp <- log_posterior(fits.diag[["seq"]])
    ## np <- nuts_params(fits.diag[["seq"]])
    ## p <- grid.arrange(mcmc_nuts_divergence(np, lp),top="mcmc_nuts_divergence: regular + diag_e")
    ## print(p)
    ## lp <- log_posterior(fits.dense[["mpi"]])
    ## np <- nuts_params(fits.dense[["mpi"]])
    ## p <- grid.arrange(mcmc_nuts_divergence(np, lp),top="mcmc_nuts_divergence: cross-chain + dense_e")
    ## print(p)
    ## lp <- log_posterior(fits.dense[["seq"]])
    ## np <- nuts_params(fits.dense[["seq"]])
    ## p <- grid.arrange(mcmc_nuts_divergence(np, lp),top="mcmc_nuts_divergence: regular + dense_e")
    ## print(p)
    ## dev.off()

    grid.table(format(cbind(fits.diag[["summary"]], fits.dense[["summary"]]), digits=3))

    d <- cbind(fits.diag[["summary"]], fits.dense[["summary"]])
    d%>%
        tibble::rownames_to_column()%>%
        tidyr::gather('col','val',-rowname)%>%
        tidyr::separate(col,c('type','metric'),sep='\\.')%>%
        dplyr::filter(grepl('',rowname))%>%
        ggplot2::ggplot(ggplot2::aes(x=metric,y=val,fill=type)) +
        ggplot2::geom_bar(stat = 'identity',position = ggplot2::position_dodge()) +
        ggplot2::facet_wrap(~rowname,scales='free')

    ggplot2::ggsave(file=paste0(paste0(model, "_cross_chain_summary.png")))

    return(list(summary=cbind(fits.diag[["summary"]], fits.dense[["summary"]]),
                mpi.diag=fits.diag[["mpi"]],
                seq.diag=fits.diag[["seq"]],
                mpi.dense=fits.dense[["mpi"]],
                seq.dense=fits.dense[["seq"]]))
}
