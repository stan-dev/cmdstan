functions {


/*
 * Author: S. Weber
 *
 * Various utlities
 */

// turn a slicing variable for a ragged array
// S = {5, 6, 11}
// into
// Si = {0, 5, 5 + 6, 5+6+11} + 1
// such that we can index the ragged array A as
// A[Si[u] : Si[u+1]-1]
// for the uth unit
int[] make_slice_index(int[] S) {
  int Si[size(S)+1];
  int cv = 1;
  Si[1] = cv;
  for(i in 1:size(S)) {
    cv = cv + S[i];
    Si[i+1] = cv;
  }
  return(Si);
}

// helper function for rle_int
int rle_elem_count(int[] set) {
  int U = 1;
  for(i in 2:num_elements(set)) {
    if(set[i-1] != set[i])
      U = U + 1;
  }
  return(U);
}

// helper function for rle_int
int rle_elem_count_vector(vector set) {
  int U = 1;
  for(i in 2:num_elements(set)) {
    if(set[i-1] != set[i])
      U = U + 1;
  }
  return(U);
}

// repeated length encoding, see rle in R
// this function returns the # of times elements are repeated ...
int[] rle_int(int[] set) {
  int res[rle_elem_count(set)];
  int c = 1;
  res[1] = 1;
  for(i in 2:num_elements(set)) {
    if(set[i-1] == set[i]) {
      res[c] = res[c] + 1;
    } else {
      c = c + 1;
      res[c] = 1;
    }
  }
  return(res);
}

// ... and this function returns which elements are repeated for ints
int[] rle_elem_int(int[] set) {
  int N = rle_elem_count(set);
  int first_ind[N] = make_slice_index(rle_int(set))[1:N];

  return set[first_ind];
}

/* checks if in an id vector a given id is used in multiple blocks. If
   so, a reject is fired to ensure that the id vector is blocked into
   sorted groups.
 */
void check_duplicate_ids(int[] id) {
  int N = rle_elem_count(id);
  int sorted_ids[N] = sort_asc(rle_elem_int(id));
  int cid = sorted_ids[1];
  for(i in 1:N-1) {
    if(sorted_ids[i] == sorted_ids[i+1])
      reject("ID ", sorted_ids[i], " occurs multiple times within id vector.");
  }
}

int[] decimal2base(int decimal, int digits, int base) {
  int base_rep[digits];
  int current = decimal;
  for(i in 1:digits) {
    base_rep[i] = current % base;
    current = current / base;
  }
  return base_rep;
}

int power_int(int number, int power);
  
int power_int(int number, int power) {
  if(power < 0)
    reject("Cannot raise an integer to a negative power and expect an integer result.");
  if (power == 0)
    return 1;
  else
    return number * power_int(number, power - 1);
}

// count the number of unique elements
int cardinality_int(int[] elems) {
  return rle_elem_count(sort_asc(elems));
}

// count the number of unique elements
int cardinality_vector(vector elems) {
  return rle_elem_count_vector(sort_asc(elems));
}

// create an integer sequence
int[] seq_int(int start, int end) {
  int N = end - start + 1;
  int seq[N];
  for(i in 1:N) seq[i] = i + start - 1;
  return(seq);
}

// return an int vector with each element in the input repeated as
// many times as given in the each argument
int[] rep_each(int[] set, int[] each) {
  int N = sum(each);
  int replicated[N];
  int p = 1;

  for(i in 1:size(set)) {
    replicated[p:p+each[i]-1] = rep_array(set[i], each[i]);
    p = p + each[i];
  }

  return(replicated);
}

/* calculate the absolute value of a - b in log-space with log(a)
   and log(b) given. Does so by squaring and taking the root, i.e.
   
   la = log(a)
   lb = log(b)
   
   sqrt( (a - b)^2 ) = sqrt( a^2 - 2 * a * b + b^2 )
   
   <=> 0.5 * log_diff_exp(log_sum_exp(2*la, 2*lb), log(2) + la + lb)
*/
real log_diff_exp_abs(real la, real lb) {
  return(0.5 * log_diff_exp(log_sum_exp(2*la, 2*lb), log(2) + la + lb));
}


/* find_interval, see findInterval from R
 * i.e. find the ranks of x in sorted; sorted is assumed to be weakly-sorted.
 */
int[] find_interval_slow(vector x, vector sorted) {
  int res[num_elements(x)];
  // very brutal and ineffcient way of doing this, but well, it's
  // C++ speed...
  for(i in 1:num_elements(x)) {
    res[i] = rank(append_row(rep_vector(x[i], 1), sorted), 1);
  }
  return(res);
}

/* faster version which uses bisectioning search
 */
int find_interval_elem(real x, vector sorted, int start_ind) {
  int res;
  int N;
  int max_iter;
  real left;
  real right;
  int left_ind;
  int right_ind;
  int iter;
    
  N = num_elements(sorted);
  
  if(N == 0) return(0);
  
  left_ind  = start_ind;
  right_ind = N;
  
  max_iter = 100 * N;
  left  = sorted[left_ind ] - x;
  right = sorted[right_ind] - x;
  
  if(0 <= left)  return(left_ind-1);
  if(0 == right) return(N-1);
  if(0 >  right) return(N);
  
  iter = 1;
  while((right_ind - left_ind) > 1  && iter != max_iter) {
    int mid_ind;
    real mid;
    // is there a controlled way without being yelled at with a
    // warning?
    mid_ind = (left_ind + right_ind) / 2;
    mid = sorted[mid_ind] - x;
    if (mid == 0) return(mid_ind-1);
    if (left  * mid < 0) { right = mid; right_ind = mid_ind; }
    if (right * mid < 0) { left  = mid; left_ind  = mid_ind; }
    iter = iter + 1;
  }
  if(iter == max_iter)
    print("Maximum number of iterations reached.");
  return(left_ind);
}

int[] find_interval(vector x, vector sorted) {
  int res[num_elements(x)];
  for(i in 1:num_elements(x)) {
    res[i] = find_interval_elem(x[i], sorted, 1);
  }
  return(res);
}

// takes as input x an ascending sorted vector x which allows to
// move the left starting index to be moved
int[] find_interval_asc(vector x, vector sorted) {
  int res[num_elements(x)];
  int last;
  last = 1;
  for(i in 1:num_elements(x)) {
    res[i] = find_interval_elem(x[i], sorted, last);
    if(res[i] > 0) last = res[i];
  }
  return(res);
}

int[] find_interval_blocked(int[] vals_M, vector vals, int[] sorted_M, vector sorted) {
  int res[num_elements(vals)];
  int M;
  int v;
  int s;
  M = num_elements(vals_M);
  v = 1;
  s = 1;
  for(m in 1:M) {
    int temp[vals_M[m]];
    temp = find_interval(segment(vals, v, vals_M[m]), segment(sorted, s, sorted_M[m]));
    for(n in 1:vals_M[m])
      res[v + n - 1] = temp[n];
    v = v + vals_M[m];
    s = s + sorted_M[m];
  }
  return(res);
}

// count number times elem appears in test set
int count_elem(int[] test, int elem) {
  int count;
  count = 0;
  for(i in 1:num_elements(test))
    if(test[i] == elem)
      count = count + 1;
  return(count);
}

// count number times elems appears in test set
int[] count_elems(int[] test, int[] elems) {
  int counts[num_elements(elems)];
  for(i in 1:num_elements(elems))
    counts[i] = count_elem(test, elems[i]);
  return(counts);
}

// find elements in test which are equal to elem
int[] which_elem(int[] test, int elem) {
  int res[count_elem(test, elem)];
  int ci;
  ci = 1;
  for(i in 1:num_elements(test))
    if(test[i] == elem) {
      res[ci] = i;
      ci = ci + 1;
    }
  return(res);
}

// divide fac by div and return the rounded down integer
int floor_div_int(real fac, real div) {
  int count;
  if(fac < 0)
    reject("floor_div_int only works for positive values.");
  count = 1;
  while(count * div <= fac) { count = count + 1; }
  count = count - 1;
  return count;
}

int[] count_obs_event_free(int[] obs_timeRank, int ndose) {
  int dose_next_obs[ndose];
  int o;
  int O;
  dose_next_obs = rep_array(0, ndose);
  o = 0;
  O = size(obs_timeRank);
  while (o < O && obs_timeRank[o+1] == 0) { o = o + 1; }
  for (i in 1:ndose) {
    int count;
    count = 0;
    while(o < O && obs_timeRank[o+1] == i) {
      o = o + 1;
      count = count + 1;
    }
    dose_next_obs[i] = count;
  }
  return(dose_next_obs);
}

int[] count_obs_event_free_blocked(int[] M, int[] obs_timeRank, int[] ndose) {
  int dose_next_obs[sum(ndose)];
  int l;
  int ld;
  dose_next_obs = rep_array(0, sum(ndose));
  l = 1;
  ld = 1;
  for (i in 1:size(M)) {
    int u;
    int ud;
    u = l + M[i] - 1;
    ud = ld + ndose[i] - 1;
    dose_next_obs[ld:ud] = count_obs_event_free(obs_timeRank[l:u], ndose[i]);
    l = u + 1;
    ld = ud + 1;
  }
  return(dose_next_obs);
}

/*
 * subsets the input data structure to the indices given as second
 * argument
 */
int[] subset_int(int[] cand, int[] ind_set) {
  int out[size(ind_set)];
  for(i in 1:size(ind_set))
    out[i] = cand[ind_set[i]];
  return out;
}
  
vector subset_vec(vector cand, int[] ind_set) {
  vector[size(ind_set)] out;
  for(i in 1:size(ind_set))
    out[i] = cand[ind_set[i]];
  return out;
}
  
matrix subset_matrix(matrix cand, int[] ind_set) {
  matrix[size(ind_set),cols(cand)] out;
  for(i in 1:size(ind_set))
    out[i] = cand[ind_set[i]];
  return out;
}
  
// check that assumption of 1...J labeling of rows hold and warn if
// not
void check_ids(int[] id) {
  int cid = 0;
  int warned = 0;
  cid = 0;
  for(n in 1:num_elements(id)) {
    if(id[n] != cid) {
      if(id[n] != cid + 1) {
        if(!warned)
          print("WARNING: id vector not correctly sorted, i.e. not in range 1..J. Consider using the cid vector internally.");
        warned = 1;
      } else {
        cid = cid + 1;
      }
    }
  }
  if(max(id) != cid)
    print("WARNING: Last patient's id not equal to max(id).");
}

// check that addl dose coding is correct. That is, we can only have a
// single active addl dose such that addl dosing must be off whenever
// the next dose occurs.
void check_addl_dosing(vector dose_time, vector dose_tau, int[] dose_addl) {
  int D = num_elements(dose_time);

  for(d in 2:D) {
    if(dose_time[d] < (dose_time[d-1] + dose_tau[d-1] * dose_addl[d-1]))
      reject("Forbidden overlapping dosing records found.");
  }
}

void check_addl_dosing_blocked(int[] dose_M, vector dose_time, vector dose_tau, int[] dose_addl) {
  int M = num_elements(dose_M);
  int b = 1;
  for(m in 1:M) {
    check_addl_dosing(segment(dose_time, b, dose_M[m]),
                      segment(dose_tau, b, dose_M[m]),
                      segment(dose_addl, b, dose_M[m]));
    b = b + dose_M[m];
  }
}

  
  real tau_prior_lpdf(real tau, int dist, real a, real b) {
    if (dist == 0) {
      // this is the fixed density => we assign standard normals to
      // avoid sampling issues
      return normal_lpdf(tau| 0, 1);
    } else if (dist == 1) {
      return lognormal_lpdf(tau| a, b);
    } else if (dist == 2) {
      return normal_lpdf(tau| a, b);
    }
    reject("Invalid distribution for tau.");
    return 0.0;
  }
  
  // create for a bivariate normal it's lower triangle cholesky factor
  matrix bvn_cholesky_lower(vector tau, real rho) {
    return [ [tau[1], 0.0], [tau[2] * rho, tau[2] * sqrt(1.0-rho^2)]];
  }

  /* given design matrices and parameters calculates the logit. The
   * number of tries n is only passed to sort out n=0 cases.
   */
  vector blrm_logit_fast(int[] obs_gidx,
                         int[] n,
                         matrix[] X_comp, int[,] finite_cov,
                         matrix X_inter, 
                         vector[] beta, vector eta) {
    int num_obs = size(obs_gidx);
    int num_comp = size(X_comp);
    int num_inter = cols(X_inter);
    vector[num_obs] mu;

    for(i in 1:num_obs) {
      int idx = obs_gidx[i];
      real log_p0_nr = 0.0;
      if(n[idx] == 0) {
        // in case of no observation we merely fill in 0 as a dummy
        mu[i] = 0.0;
      } else {
        for(j in 1:num_comp) {
          // ensure that input is finite
          if(finite_cov[j,idx]) {
            log_p0_nr += log_inv_logit( -1.0 * X_comp[j,idx] * beta[j] );
          }
        }
        // turn log(1-p0) into a logit for p0
        mu[i] = log1m_exp(log_p0_nr) - log_p0_nr;
      }
      
      // add interaction part
      if(num_inter > 0) {
        mu[i] += X_inter[idx] * eta;
      }
    }
    
    return mu;
  }

  // convenience version which creates finite_cov on the fly (for
  // prediction)
  /*
  vector blrm_logit(int[] n,
                    matrix[] X_comp,
                    matrix X_inter, 
                    vector[] beta, vector eta) {
    int num_obs = rows(X_inter);
    int num_comp = size(X_comp);
    int finite_cov[num_comp,num_obs];

    for(j in 1:num_comp)
      for(i in 1:num_obs) {
        finite_cov[j,i] = !is_inf(X_comp[j,i,1]) && !is_inf(X_comp[j,i,2]) ? 1 : 0;
      }

    return blrm_logit_fast(n, X_comp, finite_cov, X_inter, beta, eta);
  }
  */
  
  real blrm_lpmf(int[] r,
                 int[] obs_gidx, int[] n,
                 matrix[] X_comp, int[,] finite_cov,
                 matrix X_inter,
                 vector[] beta, vector eta) {
    int num_obs = size(obs_gidx);
    int r_obs[num_obs];
    int n_obs[num_obs];
    for(i in 1:num_obs) {
      r_obs[i] = r[obs_gidx[i]];
      n_obs[i] = n[obs_gidx[i]];
    }
    return binomial_logit_lpmf(r_obs | n_obs, blrm_logit_fast(obs_gidx, n, X_comp, finite_cov, X_inter, beta, eta) );
  }

  // calculates for a given group all mixture configurations and 
  vector blrm_mix_lpmf_comp(int g, int num_groups,
                            int[] obs_gidx,
                            int[] r, int[] n,
                            matrix[] X_comp, int[,] finite_cov,
                            matrix X_inter,
                            vector[,] beta, int[,] mix_idx_beta,
                            vector[] eta, int[,] mix_idx_eta) {
    int num_mix_comp = size(mix_idx_beta);
    int num_comp = dims(mix_idx_beta)[2];
    int num_inter = dims(mix_idx_eta)[2];
    vector[num_mix_comp] mix_lpmf;

    if (num_elements(r) == 0)
      return rep_vector(0.0, num_mix_comp);
    
    for(m in 1:num_mix_comp) {
      int ind_beta[num_comp] = mix_idx_beta[m];
      int ind_eta[num_inter] = mix_idx_eta[m];
      vector[2] beta_mix_config[num_comp];
      vector[num_inter] eta_mix_config;
      for(i in 1:num_comp)
        beta_mix_config[i] = beta[ind_beta[i] == 1 ? g : g + num_groups,i];
      for(i in 1:num_inter)
        eta_mix_config[i] = eta[ind_eta[i] == 1 ? g : g + num_groups,i];

      mix_lpmf[m] = blrm_lpmf(r | obs_gidx,
                              n,
                              X_comp,
                              finite_cov,
                              X_inter,
                              beta_mix_config,
                              eta_mix_config);
    }

    return mix_lpmf;
  }
}
data {
  // input data is by default given in row-ordered per observation
  // format. Data should be sorted by stratum / group nesting
  int<lower=0> num_obs;
  int<lower=0> r[num_obs];
  int<lower=0> nr[num_obs];

  // number of components
  int<lower=1> num_comp;
  
  // for now we fix the number of regressors to two!
  matrix[num_obs,2] X_comp[num_comp];

  // interactions
  int<lower=0> num_inter;
  matrix[num_obs,num_inter] X_inter;

  // data for parameter model
  
  // observation to group mapping
  int<lower=1> group[num_obs];

  // observation to stratum mapping (groups are nested into strata)
  int<lower=1> stratum[num_obs];

  // number of groups
  int<lower=1> num_groups;

  // number of strata
  int<lower=1> num_strata;

  // mapping of group_id to stratum (the index is the group id)
  int<lower=1,upper=num_strata> group_stratum_cid[num_groups];
  
  // definition which parameters are robust
  int<lower=0,upper=1> prior_is_EXNEX_comp[num_comp];
  int<lower=0,upper=1> prior_is_EXNEX_inter[num_inter];

  // per group probability for EX for each parameter which is modelled
  // as robust EX/NEX
  
  // for now enforce a minimal EX to avoid issues with 0: Todo
  // note: we define these prior probabilities even if not used
  matrix<lower=1E-6,upper=1>[num_groups,num_comp] prior_EX_prob_comp;
  matrix<lower=1E-6,upper=1>[num_groups,num_inter] prior_EX_prob_inter;

  // EX priors
  vector[2] prior_EX_mu_mean_comp[num_comp];
  vector<lower=0>[2] prior_EX_mu_sd_comp[num_comp];
  vector[2] prior_EX_tau_mean_comp[num_strata,num_comp];
  vector<lower=0>[2] prior_EX_tau_sd_comp[num_strata,num_comp];
  real<lower=0> prior_EX_corr_eta_comp[num_comp];
  vector[num_inter] prior_EX_mu_mean_inter;
  vector<lower=0>[num_inter] prior_EX_mu_sd_inter;
  vector[num_inter] prior_EX_tau_mean_inter[num_strata];
  vector<lower=0>[num_inter] prior_EX_tau_sd_inter[num_strata];
  real<lower=0> prior_EX_corr_eta_inter;

  // NEX priors (same for each group)
  vector[2] prior_NEX_mu_mean_comp[num_comp];
  vector<lower=0>[2] prior_NEX_mu_sd_comp[num_comp];
  vector[num_inter] prior_NEX_mu_mean_inter;
  vector<lower=0>[num_inter] prior_NEX_mu_sd_inter;

  // prior distribution of tau's
  // 0 = fixed to its mean
  // 1 = log-normal
  // 2 = truncated normal
  int<lower=0,upper=2> prior_tau_dist;
  
  // sample from prior predictive (do not add data to likelihood)
  int<lower=0,upper=1> prior_PD;
}
transformed data {
  int<lower=0> n[num_obs];
  int<lower=0,upper=1> finite_cov[num_comp,num_obs];
  int<lower=0,upper=num_comp> num_EXNEX_comp = sum(prior_is_EXNEX_comp);
  int<lower=0,upper=num_inter> num_EXNEX_inter = sum(prior_is_EXNEX_inter);
  int<lower=0> num_mix_dim = num_EXNEX_comp + num_EXNEX_inter;
  int<lower=0> num_mix_comp = power_int(2, num_mix_dim);
  int<lower=1,upper=num_mix_comp> mix_is_EX_beta[num_EXNEX_comp,num_mix_dim == 0 ? 0 : power_int(2, num_mix_dim-1)];
  int<lower=1,upper=num_mix_comp> mix_is_EX_eta[num_EXNEX_inter,num_mix_dim == 0 ? 0 : power_int(2, num_mix_dim-1)];
  int<lower=1,upper=2> mix_idx_beta[num_mix_comp,num_comp];
  int<lower=1,upper=2> mix_idx_eta[ num_mix_comp,num_inter];
  // number of observations per group
  int<lower=0,upper=num_obs> num_obs_group[num_groups] = count_elems(group, seq_int(1, num_groups));
  // number of cases per group
  int<lower=0> num_cases_group[num_groups] = rep_array(0, num_groups);
  // indices for each group
  int<lower=0,upper=num_obs> group_obs_idx[num_groups,max(num_obs_group)] = rep_array(0, num_groups, max(num_obs_group));
  vector<upper=0>[num_mix_comp] mix_log_weight[num_groups];

  // determine for each group the set of indices which belong to it
  for (g in 1:num_groups) {
    int i = 1;
    for (o in 1:num_obs) {
      if(group[o] == g) {
        group_obs_idx[g,i] = o;
        i = i + 1;
      }
    }
  }

  // check that within each group the stratum does not change which
  // would violate the nested structure
  for(g in 1:num_groups) {
    int group_size = num_obs_group[g];
    int obs_gidx[group_size] = group_obs_idx[g,1:group_size];
    if(cardinality_int(stratum[obs_gidx]) > 1)
      reject("Group ", g, " is assigned to multiple strata.");
  }
  
  for(j in 1:num_comp) {
    if(cardinality_vector(X_comp[j,:,1]) > 1 || X_comp[j,1,1] != 1.0)
      reject("Compound (", j, ") design matrix must have an intercept.");
  }

  if(num_inter > 0)
    if(cardinality_vector(X_inter[:,1]) == 1 && X_inter[1,1] == 1.0)
      print("INFO: Interaction design matrix appears to have an intercept, which is unexpected.");

  // NOTE: Non-centered parametrization is hard-coded

  for(i in 1:num_obs)
    n[i] = r[i] + nr[i];

  // count number of cases per group
  for(g in 1:num_groups) {
    int group_size = num_obs_group[g];
    int obs_gidx[group_size] = group_obs_idx[g,1:group_size];
    num_cases_group[g] = sum(n[obs_gidx]);
  }
  
  {
    int finite_cov_sum[num_obs] = rep_array(0, num_obs);
    for(j in 1:num_comp) {
      for(i in 1:num_obs) {
        finite_cov[j,i] = !is_inf(X_comp[j,i,1]) && !is_inf(X_comp[j,i,2]) ? 1 : 0;
        finite_cov_sum[i] = finite_cov_sum[i] + finite_cov[j,i];
      }
    }
    for(i in 1:num_obs) {
      if(finite_cov_sum[i] == 0)
        reject("No finite covariates for observation ", i);
    }
  }

  print("Number of groups: ", num_groups);
  print("Number of strata: ", num_strata);
  print("EXNEX enabled for compounds ", num_EXNEX_comp, "/", num_comp, ":    ", prior_is_EXNEX_comp);
  print("EXNEX enabled for interactions ", num_EXNEX_inter, "/", num_inter, ": ", prior_is_EXNEX_inter);
  print("EXNEX mixture dimensionality ", num_mix_dim, " leads to ", num_mix_comp, " combinations.");

  print("Observation => group assignment:");
  for(g in 1:num_groups) {
    print("Group ", g, ": ", group_obs_idx[g,1:num_obs_group[g]]);
  }

  print("");
  print("Group => stratum assignment:");
  for(g in 1:num_groups) {
    print(g, " => ", group_stratum_cid[g]);
  }
  
  print("Prior distribution on tau parameters:");
  if(prior_tau_dist == 0) {
    print("Fixed");
  } else if (prior_tau_dist == 1) {
    print("Log-Normal");
  } else if (prior_tau_dist == 2) {
    print("Truncated Normal");
  }
  
  if (prior_PD)
    print("Info: Sampling from prior predictive distribution.");
  
  for(g in 1:num_groups)
    mix_log_weight[g] = rep_vector(0.0, num_mix_comp);

  // here we configure the different mixture possibilities since we
  // are not exhaustivley using all combinations those elements which
  // are fixed are filled in as a second step.
  for(i in 1:num_mix_comp) {
    int mix_ind_base[num_mix_dim] = decimal2base(i-1, num_mix_dim, 2);
    int mix_ind[num_comp + num_inter];
    for(j in 1:num_mix_dim) {
      // move 0/1 coding to 1/2
      mix_ind_base[j] += 1;
    }

    {
      int k = 1;
      for(j in 1:num_comp) {
        // if the component is EXNEX, then its status is controlled by
        // the moving configuration; otherwise it is always EX
        if (prior_is_EXNEX_comp[j]) {
          mix_ind[j] = mix_ind_base[k];
          k += 1;
        } else {
          mix_ind[j] =  1;
        }
      }
    }
    {
      int k = 1;
      for(j in 1:num_inter) {
        // if the interaction is EXNEX, then its status is controlled by
        // the moving configuration; otherwise it is always EX
        if (prior_is_EXNEX_inter[j]) {
          mix_ind[num_comp + j] = mix_ind_base[num_EXNEX_comp + k];
          k += 1;
        } else {
          mix_ind[num_comp + j] = 1;
        }
      }
    }
    
    for(g in 1:num_groups) {
      // EX == 1 / NEX == 2
      // prior weights
      for(j in 1:num_comp)
        if(prior_is_EXNEX_comp[j])
          mix_log_weight[g,i] += mix_ind[j] == 1 ? log(prior_EX_prob_comp[g,j]) : log1m(prior_EX_prob_comp[g,j]);
      for(j in 1:num_inter)
        if(prior_is_EXNEX_inter[j])
          mix_log_weight[g,i] += mix_ind[num_comp + j] == 1 ? log(prior_EX_prob_inter[g,j]) : log1m(prior_EX_prob_inter[g,j]);

      // index configuration
      mix_idx_beta[i] = mix_ind[1:num_comp];
      mix_idx_eta[i] = mix_ind[num_comp+1:num_comp+num_inter];
    }
  }

  // index vectors which indicate whenever a given parameter is EX for
  // the configuration number
  {
    int i = 1;
    for(j in 1:num_comp) {
      if (prior_is_EXNEX_comp[j]) {
        mix_is_EX_beta[i] = which_elem(mix_idx_beta[:,j], 1);
        i += 1;
      }
    }
  }
  {
    int i = 1;
    for(j in 1:num_inter) {
      if (prior_is_EXNEX_inter[j]) {
        mix_is_EX_eta[i] = which_elem(mix_idx_eta[:,j], 1);
        i += 1;
      }
    }
  }
}
parameters {
  // the first 1:num_groups parameters are EX modelled while the
  // num_groups+1:2*num_groups are NEX
  vector[2] log_beta_raw[2*num_groups,num_comp];
  vector[num_inter] eta_raw[2*num_groups];

  // hierarchical priors
  vector[2] mu_log_beta[num_comp];
  // for differential discounting we allow the tau's to vary by
  // stratum (but not the means)
  vector<lower=0>[2] tau_log_beta_raw[num_strata,num_comp];
  cholesky_factor_corr[2] L_corr_log_beta[num_comp];

  vector[num_inter] mu_eta;
  vector<lower=0>[num_inter] tau_eta_raw[num_strata];
  cholesky_factor_corr[num_inter] L_corr_eta;
}
transformed parameters {
  vector[2] beta[2*num_groups,num_comp];
  vector[num_inter] eta[2*num_groups];
  vector<lower=0>[2] tau_log_beta[num_strata,num_comp];
  vector<lower=0>[num_inter] tau_eta[num_strata];
  
  // in the case of fixed tau's we fill them in here
  if (prior_tau_dist == 0) {
    tau_log_beta = prior_EX_tau_mean_comp;
    tau_eta = prior_EX_tau_mean_inter;
  } else {
    tau_log_beta = tau_log_beta_raw;
    tau_eta = tau_eta_raw;
  }

  // EX parameters which vary by stratum which is defined by the group
  for(g in 1:num_groups) {
    int s = group_stratum_cid[g];
    for(j in 1:num_comp) {
      beta[g,j] = mu_log_beta[j] +
        diag_pre_multiply(tau_log_beta[s,j], L_corr_log_beta[j]) * log_beta_raw[g,j];
    }
    if (num_inter > 0)
      eta[g] = mu_eta + diag_pre_multiply(tau_eta[s], L_corr_eta) * eta_raw[g];
  }

  // NEX parameters
  beta[num_groups+1:2*num_groups] = log_beta_raw[num_groups+1:2*num_groups];
  eta[num_groups+1:2*num_groups] = eta_raw[num_groups+1:2*num_groups];

  // exponentiate the slope parameter to force positivity
  for(g in 1:2*num_groups)
    for(j in 1:num_comp)
      beta[g,j,2] = exp(beta[g,j,2]);
}
model {
  real log_lik = 0.0;
  // loop over the data by group; nested into that we have to
  // loop over the different mixture configurations
  for(g in 1:num_groups) {
    int s = group_stratum_cid[g];
    int group_size = num_obs_group[g];
    int obs_gidx[group_size] = group_obs_idx[g,1:group_size];
    if(num_cases_group[g] != 0) {
      // lpmf for each mixture configuration
      vector[num_mix_comp] mix_lpmf =
          blrm_mix_lpmf_comp(// subset data
              g, num_groups,
              obs_gidx,
              r,
              n,
              X_comp,
              finite_cov,
              X_inter,
              // select EX+NEX of this group
              beta, mix_idx_beta,
              eta, mix_idx_eta)
          // prior weight for each component
          + mix_log_weight[g];
      // finally add the sum (on the natural scale) as log to the target
      // log density
      log_lik += log_sum_exp(mix_lpmf);
    } // num_cases_group[g] == 0 => log_lik = 0
  }
  if (!prior_PD)
    target += log_lik;
  
  // EX part: hyper-parameters priors for hierarchical priors
  for(j in 1:num_comp) {
    mu_log_beta[j,1] ~ normal(prior_EX_mu_mean_comp[j,1], prior_EX_mu_sd_comp[j,1]);
    mu_log_beta[j,2] ~ normal(prior_EX_mu_mean_comp[j,2], prior_EX_mu_sd_comp[j,2]);
    for(s in 1:num_strata) {
      tau_log_beta_raw[s,j,1] ~ tau_prior(prior_tau_dist, prior_EX_tau_mean_comp[s,j,1], prior_EX_tau_sd_comp[s,j,1]);
      tau_log_beta_raw[s,j,2] ~ tau_prior(prior_tau_dist, prior_EX_tau_mean_comp[s,j,2], prior_EX_tau_sd_comp[s,j,2]);
    }
    L_corr_log_beta[j] ~ lkj_corr_cholesky(prior_EX_corr_eta_comp[j]);
  }
  
  mu_eta ~ normal(prior_EX_mu_mean_inter, prior_EX_mu_sd_inter);
  for(s in 1:num_strata)
    for(j in 1:num_inter)
      tau_eta_raw[s,j] ~ tau_prior(prior_tau_dist, prior_EX_tau_mean_inter[s,j], prior_EX_tau_sd_inter[s,j]);
  L_corr_eta ~ lkj_corr_cholesky(prior_EX_corr_eta_inter);

  
  // hierarchical priors NCP
  for(g in 1:num_groups) {
    for(j in 1:num_comp)
      log_beta_raw[g,j] ~ normal(0, 1);

    eta_raw[g] ~ normal(0, 1);
  }

  // NEX priors (always uncorrelated)
  for(g in num_groups+1:2*num_groups) {
    for(j in 1:num_comp) {
      // vectorized over intercept and slope
      log_beta_raw[g,j] ~ normal(prior_NEX_mu_mean_comp[j], prior_NEX_mu_sd_comp[j]);
    }
    eta_raw[g] ~ normal(prior_NEX_mu_mean_inter, prior_NEX_mu_sd_inter);
  }
}
generated quantities {
  matrix[num_groups,num_comp] beta_EX_prob;
  matrix[num_groups,num_inter] eta_EX_prob;
  vector[2] beta_group[num_groups,num_comp];
  vector[num_inter] eta_group[num_groups];
  vector[num_groups] log_lik_group;
  vector[num_comp] rho_log_beta;
  matrix[num_inter,num_inter] Sigma_corr_eta = multiply_lower_tri_self_transpose(L_corr_eta);

  for(j in 1:num_comp) {
    matrix[2,2] Sigma_corr_log_beta = multiply_lower_tri_self_transpose(L_corr_log_beta[j]);
    rho_log_beta[j] = Sigma_corr_log_beta[2,1];
  }

  // posterior EX weights per group
  for(g in 1:num_groups) {
    int s = group_stratum_cid[g];
    int group_size = num_obs_group[g];
    int obs_gidx[group_size] = group_obs_idx[g,1:group_size];
    // lpmf for each mixture configuration
    vector[num_mix_comp] mix_lpmf =
        blrm_mix_lpmf_comp(g, num_groups,
                           obs_gidx,
                           r,
                           n,
                           X_comp,
                           finite_cov,
                           X_inter,
                           beta, mix_idx_beta,
                           eta, mix_idx_eta)
      + mix_log_weight[g];
    real log_norm = log_sum_exp(mix_lpmf);
    vector[num_mix_comp] log_EX_prob_mix = mix_lpmf - log_norm;
    int mix_config_ind = categorical_rng(exp(log_EX_prob_mix));
    int mix_beta_config[num_comp] = mix_idx_beta[mix_config_ind];
    int mix_eta_config[num_inter] = mix_idx_eta[mix_config_ind];

    log_lik_group[g] = log_norm;
    
    // marginalize & pick group specific parameters which have been sampled
    {
      int i = 1;
      for(j in 1:num_comp) {
        if (prior_is_EXNEX_comp[j]) {
          beta_EX_prob[g,j] = exp(log_sum_exp(log_EX_prob_mix[mix_is_EX_beta[i]]));
          i += 1;
        } else {
          beta_EX_prob[g,j] = 1.0;
        }
        beta_group[g,j] = beta[g + (mix_beta_config[j] == 1 ? 0 : num_groups),j];
      }
    }
    {
      int i = 1;
      for(j in 1:num_inter) {
        if (prior_is_EXNEX_inter[j]) {
          eta_EX_prob[g,j] = exp(log_sum_exp(log_EX_prob_mix[mix_is_EX_eta[i]]));
          i += 1;
        } else {
          eta_EX_prob[g,j] = 1.0;
        }
        eta_group[g,j] = eta[g + (mix_eta_config[j] == 1 ? 0 : num_groups),j];
      }
    }
  }
} 
