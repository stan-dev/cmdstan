#ifndef CMDSTAN_ARGUMENTS_ARG_PATHFINDER_MULTI_HPP
#define CMDSTAN_ARGUMENTS_ARG_PATHFINDER_MULTI_HPP

namespace cmdstan {

class arg_pathfinder_multi : public arg_pathfinder_single {
 public:
  arg_pathfinder_multi() {
    _name = "multi";
    _description = "pathfinder with pathfinder, u know";
  }
};

}  // namespace cmdstan
#endif
