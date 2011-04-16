/** eval-table.cc --- 
 *
 * Copyright (C) 2011 Nil Geisweiller
 *
 * Author: Nil Geisweiller <nilg@desktop>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include <opencog/util/iostreamContainer.h>
#include <opencog/util/mt19937ar.h>
#include <opencog/util/numeric.h>

#include "eval-table.h"

using namespace boost::program_options;
using boost::lexical_cast;
using namespace opencog;

/**
 * Program to output the result of a combo program given input data
 * described in CSV format. It has a few additional options to
 * compute the mutual information, and more to add.
 */

/**
 * Convert a string representing a combo program in a combo_tree.
 *
 * @param combo_prog_str   the string containing the combo program
 * @param has_labels       true if the combo program has labels instead of
 * place holders (like and(#start #up) instead of and(#23 #134)
 * @param labels           a vector of labels
 * @return                 the combo_tree
 */
combo_tree str2combo_tree_label(const std::string& combo_prog_str,
                                bool has_labels,
                                const std::vector<std::string>& labels) {
    // combo pogram with place holders
    std::string combo_prog_ph_str = 
        has_labels? l2ph(combo_prog_str, labels) : combo_prog_str;
    std::stringstream ss(combo_prog_ph_str);
    combo_tree tr;
    ss >> tr;
    return tr;
}

/**
 * Program to evaluate a combo program over a data set repsented as csv file.
 */
int main(int argc,char** argv) { 

    // program options, see options_description below for their meaning
    evalTableParameters pa;
    unsigned long rand_seed;

    // Declare the supported options.
    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Produce help message.\n")
        (opt_desc_str(rand_seed_opt).c_str(),
         value<unsigned long>(&rand_seed)->default_value(1),
         "Random seed.\n")
        (opt_desc_str(input_table_opt).c_str(),
         value<string>(&pa.input_table_file),
         "Input table file.\n")
        (opt_desc_str(combo_str_opt).c_str(),
         value<vector<string> >(&pa.combo_programs),
         "Combo program to evaluate against the input table. It can be used several time so that several programs are evaluated at once.\n")
        (opt_desc_str(combo_prog_file_opt).c_str(),
         value<string>(&pa.combo_programs_file),
         "File containing combo programs to evaluate against the input table. Each combo program in the file is seperated by a new line and each results are displaied in the same order, seperated by a new line.\n")
        (opt_desc_str(labels_opt).c_str(), "If enabled then the combo program is expected to contain variables labels #labels1, etc, instead of place holders. For instance one provide the combo program \"and(#large #tall)\" instead of \"and(#24 #124)\". In such a case it is expected that the input data file contains the labels as first row.\n")
        (opt_desc_str(output_file_opt).c_str(), value<string>(&pa.output_file),
         "File where to save the results. If empty then it outputs on the stdout.\n")
        (opt_desc_str(display_output_table_opt).c_str(), value<bool>(&pa.display_output_table),
         "Display the output table resulting from applying the combo program on the input table.\n")
        ;

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if(vm.count("help") || argc == 1) {
        cout << desc << "\n";
        return 1;
    }

    // set variables
    pa.has_labels = vm.count("labels");
    // residual_error = vm.count("residual-error");

    // init random generator
    opencog::MT19937RandGen rng(rand_seed);

    // read input_table_file file
    type_node data_type = inferDataType(pa.input_table_file);

    if(data_type == id::boolean_type) {
        typedef truth_table_inputs IT;
        typedef partial_truth_table OT;
        read_eval_output_results<IT, OT, bool>(pa, rng);
    } else if(data_type == id::contin_type) {
        typedef contin_input_table IT;
        typedef contin_table OT;
        read_eval_output_results<IT, OT, contin_t>(pa, rng);
    }

}

