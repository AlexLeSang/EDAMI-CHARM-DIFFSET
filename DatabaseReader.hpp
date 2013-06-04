#ifndef DATABASEREADER_HPP
#define DATABASEREADER_HPP

#include "Typedefs.hpp"

#include "Itemset.hpp"
#include "Database.hpp"

#include <sstream>

/*!
 * \brief The DatabaseReader class
 */

template < unsigned int nfields = 0 >
class DatabaseReader
{
private:
    static constexpr unsigned int number_of_fields = nfields;

public:
    /*!
     * \brief operator ()
     * \param data_stream
     * \param database
     */
    inline void operator ()(std::ifstream & data_stream, Database & database) const
    {
        const char delim = ';';
        std::string s;
        Itemset itemset;
        Item item;
        std::string item_stirng;
        while ( ! data_stream.eof() ) {
            std::getline( data_stream, s );

            if ( !s.empty() ) {
                std::istringstream stringstream( s );
                itemset.clear();
                item_stirng.clear();
                bool first_skipped = false;
                while ( std::getline( stringstream, item_stirng, delim ) ) {
                    if ( ! first_skipped ) {
                        first_skipped = true;
                        continue;
                    }
                    std::istringstream buff( item_stirng );
                    buff >> item;
                    itemset.push_back( item );
                }
                database.push_back( itemset );
            }
        }
    }

    /*!
     * \brief read_database
     * \param data_stream
     * \param database
     */
    static void read_database(std::ifstream & data_stream, Database & database)
    {
        static DatabaseReader reader;
        reader( data_stream, database );
    }
};

#endif // DATABASEREADER_HPP
