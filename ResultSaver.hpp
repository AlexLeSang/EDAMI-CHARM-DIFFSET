#ifndef RESULTSAVER_HPP
#define RESULTSAVER_HPP

#include "CSet.hpp"
#include <iostream>
#include <fstream>

class ResultSaver
{
public:
    void operator() (std::ofstream & c_set_stream, const CSet & c_set) const
    {
        c_set_stream << c_set;
    }

    static void save(std::ofstream & c_set_stream, const CSet & c_set)
    {
        ResultSaver saver;
        saver( c_set_stream, c_set );
    }
};

#endif // RESULTSAVER_HPP
