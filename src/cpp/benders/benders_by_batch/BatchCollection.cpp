#include "antares-xpansion/benders/benders_by_batch/BatchCollection.h"

#include <cmath>
#include <iostream>

#include "antares-xpansion/benders/benders_core/BendersBase.h"

void Batch::BuildCuts(int n_cuts)
{
    cuts.reserve(n_cuts);
    SubProblemNamesInCut cut;
    cut.reserve((sub_problem_names.size() + n_cuts - 1) / n_cuts);
    for (int i = 0; i < sub_problem_names.size(); i++)
    {
        cut.push_back(std::make_pair(std::move(sub_problem_names[i]), std::move(proc_numbers[i])));
        if (cut.size() == static_cast<size_t>((sub_problem_names.size() + n_cuts - 1) / n_cuts))
        {
            cuts.push_back(std::move(cut));
            cut.clear();
            cut.reserve((sub_problem_names.size() + n_cuts - 1) / n_cuts);
        }
    }
    if (!cut.empty())
    {
        cuts.push_back(std::move(cut));
    }
    sub_problem_names.clear();
    proc_numbers.clear();
}

BatchCollection::BatchCollection(const std::vector<std::string>& sub_problem_names,
                                 size_t batch_size,
                                 Logger logger):
    sub_problem_names_(sub_problem_names),
    sub_problems_number_(sub_problem_names.size()),
    batch_size_(batch_size),
    logger_(std::move(logger))
{
}

void BatchCollection::BuildBatches(int n_procs)
{
    if (batch_size_ > sub_problems_number_)
    {
        logger_->display_message(std::string("batch_size(") + std::to_string(batch_size_)
                                   + ") can not be greater than number of subproblems ("
                                   + std::to_string(sub_problems_number_) + ")",
                                 LogUtils::LOGLEVEL::WARNING,
                                 "Benders By batch");
        logger_->display_message(std::string("Setting batch_size = number of subproblems(")
                                 + std::to_string(sub_problems_number_)
                                 + ")\nWhich means that there is only one batch!");
    }
    number_of_batch_ = std::ceil(double(sub_problems_number_) / batch_size_);
    for (auto id = 0; id < number_of_batch_ - 1; id++)
    {
        Batch b;
        b.id = id;

        for (int i = batch_size_ * id; i < batch_size_ * (id + 1); i++)
        {
            b.sub_problem_names.push_back(sub_problem_names_[i]);
            b.proc_numbers.push_back(i % n_procs);
        }

        batch_collections_.push_back(b);
    }
    Batch last;
    last.id = number_of_batch_ - 1;

    for (int i = (number_of_batch_ - 1) * batch_size_; i < sub_problem_names_.size(); i++)
    {
        last.sub_problem_names.push_back(sub_problem_names_[i]);
        last.proc_numbers.push_back(i % n_procs);
    }

    batch_collections_.push_back(last);
}
