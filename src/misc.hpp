using namespace seqan;

typedef std::unordered_map <uint32_t, std::pair<uint32_t, std::string> > TNodes;

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <utility>

template <typename TTarget, typename TString, typename TKey = uint32_t, typename TValue = uint32_t>
TTarget load_node_maps_2(TString const & filePath)
{
    TTarget result;
    std::ifstream giMap(toCString(filePath));
    TKey key;
    TValue value;
    while (giMap >> key >> value)
    {
        result[key]=value;
    }
    giMap.close();
    return result;
}

std::unordered_map <uint32_t, std::string> load_int__string_map(std::string const & filePath)
{
    std::unordered_map <uint32_t, std::string> result;
    std::ifstream nameMap(filePath);
    uint32_t key;
    std::string value,  line;

    while(std::getline(nameMap, line))
    {
        std::stringstream   linestream(line);
        linestream >> key;
        std::getline(linestream, value, '\t');
        std::getline(linestream, value, '\t');
        result[key]=value;
    }
    nameMap.close();
    return result;
}

TNodes load_node_maps(std::string const & filePath)
{
    TNodes target;
    std::ifstream nodeMap(filePath);
    std::string   line;

    while(std::getline(nodeMap, line))
    {
        std::stringstream   linestream(line);
        uint32_t key, value1;
        std::string value2;
        linestream >> key >> value1;
        std::getline(linestream, value2, '\t');
        std::getline(linestream, value2, '\t');
        target[key]=std::pair<uint32_t, std::string> (value1, value2);
    }

    nodeMap.close();
    return target;
}

// ==========================================================================
// Functions
// ==========================================================================

template <typename Type>
Type get_quantile_cut_off (std::vector<Type> v, float q)
{
    if (v.empty())
        return 0;
    Type total = std::accumulate(v.begin(), v.end(), (Type)0);
    Type cutoff = (Type)0, subTotal = (Type)0;

    std::sort (v.begin(), v.end());

    uint32_t i= v.size() - 1;
    while((float(subTotal)/total) < q && i > (Type)0)
    {
        subTotal += v[i];
        --i;
    }
    cutoff = v[i];

    return cutoff;
}


template <typename Type>
bool greaterThan (Type i,Type j)
{
    return (i>j);
}
template <typename Type>
bool lessThan (Type i,Type j)
{
    return (i<j);
}


bool is_file(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISREG(buf.st_mode);
}

bool is_dir(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

// Calculates log2 of number.
float log_2(float n)
{
    // log(n)/log(2) is log2.
    return std::log(n)/std::log(2);
}

// returns a vector after spliting a string into two chunks
std::vector<std::string> & split(const std::string &s,
                                 char delim,
                                 std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}
std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

template <typename T>
std::string numberToString ( T Number )
{
    std::stringstream ss;
    ss<<Number;
    return ss.str();
}

template <typename T>
T stringToNumber ( const std::string &Text )
{
    std::stringstream ss(Text);
    T result;
    return ss >> result ? result : 0;
}

template <typename T>
T median(std::vector<T> &v)
{
    size_t n = v.size() / 2;
    std::nth_element(v.begin(), v.begin()+n, v.end());
    if (n*2 == v.size())
        return v[n];
    else
        return (v[n] + v[n+1])/2.0;
}

template <typename T>
T mean(std::vector<T> &v) {
    T vSum = std::accumulate(v.begin(), v.end(), (T)0);
    return vSum/v.size();
}

template <typename T>
T variance(std::vector<T> &v, T m)
{
    T temp = 0;
    for(auto i : v)
    {
        temp += (i - m) * (i - m) ;
    }
    return temp / v.size();
}

template <typename T>
T variance(std::vector<T> &v)
{
    T m = mean(v);
    return variance(v, m);
}

template <typename T>
T stdDev(std::vector<T> &v, T m)
{
    return sqrt(variance(v, m));
}

template <typename T>
T stdDev(std::vector<T> &v)
{
    T m = mean(v);
    return sqrt(variance(v, m));
}


float calculateAlignmentScore(String<CigarElement<> > cigar,
                              int editDistance,
                              unsigned readLen)
{
    float score = 0.0;
    score += editDistance;
    typedef Iterator<String<CigarElement<> > >::Type TCigarIterator;

    for (TCigarIterator it = begin(cigar) ; it != end(cigar); goNext(it))
    {
        if (value(it).operation == 'D' || value(it).operation == 'I')
            score += float(value(it).count);
    }

    return score/readLen;
}

std::string get_file_name (const std::string& str)
{
    std::size_t found = str.find_last_of("/\\");
    return str.substr(found+1);
}

std::string get_directory (const std::string& str)
{
    std::size_t found = str.find_last_of("/\\");
    return str.substr(0,found);
}

std::string get_tsv_file_name (const std::string & oPrefix, const std::string& inpfName)
{
    std::string result = get_directory(oPrefix);
    result.append("/");
    if (length(get_file_name(oPrefix)) > 0)
        result.append(get_file_name(oPrefix));
    else
        result.append(get_file_name(inpfName));
    if ((result.find(".sam") != std::string::npos && result.find(".sam") == result.find_last_of(".")) ||
        (result.find(".bam") != std::string::npos && result.find(".bam")  == result.find_last_of(".")))
        result.replace((result.find_last_of(".")), 4, "");
    result.append(".tsv");
    return result;
}

std::string get_tsv_file_name (const std::string& oPrefix, const std::string& inpfName, const std::string& rank)
{
    std::string fName = get_tsv_file_name(oPrefix, inpfName);
    std::string sfx = "_" + rank + "_reported";
    return fName.insert(fName.size()-4, sfx);
}


// ----------------------------------------------------------------------------
// Function setDateAndVersion()
// ----------------------------------------------------------------------------

void setDateAndVersion(ArgumentParser & parser)
{
    setDate(parser, __DATE__);
    setCategory(parser, "Metagenomics");

#if defined(SEQAN_APP_VERSION)
    setVersion(parser, SEQAN_APP_VERSION);
#endif
}

// ----------------------------------------------------------------------------
// Function setDescription()
// ----------------------------------------------------------------------------

void setDescription(ArgumentParser & parser)
{
    addDescription(parser, "SLIMM  Species Level Identification of Microbes from Metagenomes");
    addDescription(parser, "See \\fI http://www.seqan.de/projects/slimm \\fP for more information.");
    addDescription(parser, "(c) Copyright 2014-2017  by Temesgen H. Dadi.");
}

typedef std::unordered_map <uint32_t, std::pair<uint32_t, std::string> > TNodes;
uint32_t getLCA(std::set<uint32_t> const & taxon_ids, std::set<uint32_t> const & valTaxaIDs, TNodes const & nodes)
{
    //consider only those under validTaxaIDs
    std::set<uint32_t> parents;
    for (auto tID : taxon_ids)
    {
        if (valTaxaIDs.find(tID) != valTaxaIDs.end())
            parents.insert(tID);
    }
    while (parents.size() > 1)
    {
        std::set<uint32_t>::iterator it = parents.begin();
        uint32_t t1 = *it;
        ++it;
        uint32_t t2 = *it;
        bool found = false;
        while (nodes.count(t1) == 1 && t1 != 0)
        {
            t2 = *it;
            while (nodes.count(t2) == 1 && t2 != 0)
            {
                if (t1 == t2)
                {
                    found = true;
                    break;
                }
                t2 = nodes.at(t2).first;
            }
            if (found)
            {
                break;
            }
            t1 = nodes.at(t1).first;
        }
        if (found)
        {
            parents.erase(parents.begin(), std::next(it));
            parents.insert(t1);
        }
        else
        {
            return 0;
        }
    }
    return *(parents.begin());
}

bool get_taxon_id(uint32_t &idPosition, CharString reference_name, std::string idType)
{
    StringSet <CharString> chunks;
    strSplit(chunks, reference_name, EqualsChar<'|'>());
    //check for slimm taxid
    for (uint32_t i = 0; i <  length(chunks) ; ++i)
    {
        if (chunks[i] == idType)
        {
            idPosition = i + 1;
            return true;
        }
    }
    return false;
}

uint32_t getLCA(std::set<uint32_t> const & taxon_ids, TNodes const & nodes)
{
    return getLCA(taxon_ids, taxon_ids, nodes);
}



uint32_t getLCA(std::vector<uint32_t> const & taxon_ids, TNodes const & nodes)
{
    std::set<uint32_t> s(taxon_ids.begin(), taxon_ids.end());
    if (s.size() == 1)
        return taxon_ids[0];
    else
        return getLCA(s, s, nodes);
}



// try to open sam file
inline bool read_bam_file(BamFileIn & bam_file, BamHeader & bam_header, std::string const & bam_file_path)
{
    if (!open(bam_file, toCString(bam_file_path)))
    {
        std::cerr << "Could not open " << bam_file_path << "!\n";
        return false;
    }
    readHeader(bam_header, bam_file);
    return true;
}

inline uint32_t get_bin_width_from_sample(BamFileIn & bam_file)
{
    BamAlignmentRecord record;
    uint32_t count = 0, totlaLength = 0;
    while (!atEnd(bam_file) && count < 1000)
    {
        readRecord(record, bam_file);
        totlaLength += length(record.seq);
        ++count;
    }
    return totlaLength/count;
}

inline uint32_t get_taxon_id_pos(CharString const & reference_name)
{
    uint32_t taxon_id_pos = 0;
    if (!get_taxon_id(taxon_id_pos, reference_name, "ti"))
    {
        if (!get_taxon_id(taxon_id_pos, reference_name, "kraken:taxid"))
        {
            std::cerr<<"Unable to find a way to resolve taxon id associated with references.\n"
            <<"Make sure you used a set of references provided with SLIMM\n"
            <<"or generated by the preprocessing script.\n";
            exit(1);
        }
    }
    return taxon_id_pos;
}



std::vector<std::string> get_bam_files_in_directory(std::string directory)
{
    std::vector<std::string>  input_paths;
#ifdef WINDOWS
    HANDLE dir;
    WIN32_FIND_DATA file_data;

    if ((dir = FindFirstFile((directory + "/*").c_str(),
                             &file_data)) == INVALID_HANDLE_VALUE)
        return; /* No files found */

    do
    {
        const std::string file_name = file_data.cFileName;
        const std::string full_file_name = directory + "/" + file_name;
        const bool is_directory =
            (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (file_name[0] == '.')
            continue;

        if (is_directory)
            continue;

        if((full_file_name.find(".sam") == full_file_name.find_last_of(".") ||
        full_file_name.find(".bam")  == full_file_name.find_last_of(".") )
           input_paths.push_back(full_file_name);
    } while (FindNextFile(dir, &file_data));

    FindClose(dir);
#else
    DIR *dir;
    struct dirent *ent;
    struct stat st;

    dir = opendir(directory.c_str());
    while ((ent = readdir(dir)) != NULL)
    {
        const std::string file_name = ent->d_name;
        const std::string full_file_name = directory + "/" + file_name;

        if (file_name[0] == '.')
            continue;

        if (stat(full_file_name.c_str(), &st) == -1)
            continue;

        const bool is_directory = (st.st_mode & S_IFDIR) != 0;

        if (is_directory)
            continue;


        if(full_file_name.find(".sam") == full_file_name.find_last_of(".") ||
        full_file_name.find(".bam")  == full_file_name.find_last_of(".") )
            input_paths.push_back(full_file_name);
        //appendValue(input_paths, full_file_name);
    }
    closedir(dir);
#endif
     return input_paths;
} // get_bam_files_in_directory

