#ifndef RESULTSAVER_HPP
#define RESULTSAVER_HPP

#include "CSet.hpp"
#include <iostream>
#include <fstream>

/*!
 * \brief The ResultSaver class
 */
class ResultSaver
{
public:
    /*!
     * \brief operator ()
     * \param c_set_stream
     * \param c_set
     */
    void operator() (std::ofstream & c_set_stream, const CSet & c_set) const
    {
        c_set_stream << c_set;
    }

    /*!
     * \brief save
     * \param c_set_stream
     * \param c_set
     */
    static void save(std::ofstream & c_set_stream, const CSet & c_set)
    {
        ResultSaver saver;
        saver( c_set_stream, c_set );
    }
};

#endif // RESULTSAVER_HPP
