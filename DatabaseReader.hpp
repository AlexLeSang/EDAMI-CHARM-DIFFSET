#ifndef DATABASEREADER_HPP
#define DATABASEREADER_HPP

#include "Typedefs.hpp"

class DatabaseReader
{
public:
    void operator ()(std::ifstream & data_stream, Database & database) const
    {
        std::string s;
        while ( ! data_stream.eof() ) {
            std::getline( data_stream, s );
            if ( s.size() ) {
                Itemset itemset( s.size() );
                std::copy( s.cbegin(), s.cend(), itemset.begin() );
                database.push_back( itemset );
            }
        }
    }

    static void read_database(std::ifstream & data_stream, Database & database)
    {
        DatabaseReader reader;
        reader( data_stream, database );
    }
};

#endif // DATABASEREADER_HPP
