// ==========================================================================
//    SLIMM - Species Level Identification of Microbes from Metagenomes.
// ==========================================================================
// Copyright (c) 2014-2017, Temesgen H. Dadi, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Temesgen H. Dadi or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL TEMESGEN H. DADI OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: Temesgen H. Dadi <temesgen.dadi@fu-berlin.de>
// ==========================================================================
#include "slimm.h"

using namespace seqan;
// --------------------------------------------------------------------------
// Function parseCommandLine()
// --------------------------------------------------------------------------
ArgumentParser::ParseResult
parseCommandLine(ArgumentParser & parser, AppOptions & options, int argc, char const ** argv)
{
    // Setup ArgumentParser.
    setDateAndVersion(parser);
    setDescription(parser);
    // Define usage line and long description.
    addUsageLine(parser, "[\\fIOPTIONS\\fP] \"\\fIIN\\fP\"");
    
    // The input file/directory argument.
    addArgument(parser,
                ArgParseArgument(ArgParseArgument::INPUT_FILE, "IN"));
    
    // The output file argument.
    addOption(parser,
              ArgParseOption("o", "output-prefix", "output path prefix.",
                             ArgParseArgument::OUTPUT_PREFIX));
    addOption(parser,
              ArgParseOption("m",
                             "mapping-files",
                             "directory containing various mapping files "
                             "(gi2taxaID.map nodes_reduced.map "
                             "names_reduced.map taxaIDFeatures.map).",
                             ArgParseOption::STRING));
    
    addOption(parser,
              ArgParseOption("w",
                             "bin-width",
                             "Set the width of a single bin in neuclotides.",
                             ArgParseArgument::INTEGER, "INT"));
    addOption(parser,
              ArgParseOption("mr",
                             "min-reads",
                             "Minimum number of matching reads to consider "
                             "a reference present.",
                             ArgParseArgument::INTEGER, "INT"));
    addOption(parser,
              ArgParseOption("r", "rank",
                             "The taxonomic rank of identification", ArgParseOption::STRING));
    setValidValues(parser, "rank", options.rankList);
    setDefaultValue(parser, "rank", options.rank);
    
    
    setDefaultValue(parser, "bin-width", options.binWidth);
    setDefaultValue(parser, "min-reads", options.minReads);
    
    addOption(parser,
              ArgParseOption("c",
                             "cov-cutoff",
                             "the quantile of coverages to use as a cutoff "
                             "smaller value means bigger threshold.",
                             ArgParseArgument::DOUBLE, "DOUBLE"));
    
    setMinValue(parser, "cov-cutoff", "0.0");
    setMaxValue(parser, "cov-cutoff", "1.0");
    setDefaultValue(parser, "cov-cutoff", options.covCutOff);
    
    addOption(parser,
              ArgParseOption("d", "directory", "Input is a directory."));
    addOption(parser,
              ArgParseOption("or", "output-raw", "Output raw reference statstics"));
    addOption(parser,
              ArgParseOption("v", "verbose", "Enable verbose output."));
    
    // Add Examples Section.
    addTextSection(parser, "Examples");
    
    addListItem(parser,
                "\\fBslimm\\fP \\fB-v\\fP \\fIexample.bam\\fP "
                "-o \\fIfeatures.tsv\\fP",
                "get the microbes present in \"example.bam\" alignment "
                "file and write the result to \"features.tsv\".");
    
    addListItem(parser, "\\fBslimm\\fP \\fB-d\\fP \\fIexample-dir\\fP "
                "-o \\fI~/results/\\fP",
                "get the microbes present in the SAM/BAM files located "
                "under \"example-dir\" and write the result to \""
                "in results folder under the home directory\".");
    
    ArgumentParser::ParseResult res = parse(parser, argc, argv);
    
    if (res != ArgumentParser::PARSE_OK)
        return res;
    
    // Extract option values.
    if (isSet(parser, "mapping-files"))
        getOptionValue(options.mappingDir, parser, "mapping-files");
    
    if (isSet(parser, "bin-width"))
        getOptionValue(options.binWidth, parser, "bin-width");
    
    if (isSet(parser, "min-reads"))
        getOptionValue(options.minReads, parser, "min-reads");
    
    if (isSet(parser, "rank"))
        getOptionValue(options.rank, parser, "rank");
    
    if (isSet(parser, "cov-cutoff"))
        getOptionValue(options.covCutOff, parser, "cov-cutoff");
    
    if (isSet(parser, "verbose"))
        getOptionValue(options.verbose, parser, "verbose");
    
    if (isSet(parser, "directory"))
        options.isDirectory = true;
    
    if (isSet(parser, "output-raw"))
        options.outputRaw = true;

    getArgumentValue(options.inputPath, parser, 0);

    getOptionValue(options.outputPrefix, parser, "output-prefix");
    if (!isSet(parser, "output-prefix"))
        options.outputPrefix = options.inputPath;

    
    return ArgumentParser::PARSE_OK;
}

// --------------------------------------------------------------------------
// Function main()
// --------------------------------------------------------------------------

// Program entry point.
int main(int argc, char const ** argv)
{
    // Parse the command line.
    ArgumentParser parser("slimm");
    AppOptions options;
    ArgumentParser::ParseResult res = parseCommandLine(parser, options, argc, argv);
    
    // If there was an error parsing or built-in argument parser functionality
    // was triggered then we exit the program.  The return code is 1 if there
    // were errors and 0 if there were none.
    
    if (res != ArgumentParser::PARSE_OK)
        return res == ArgumentParser::PARSE_ERROR;
    
    // Prepare the
    std::vector<std::string> inputPaths;
    uint32_t numFiles = 1, totalRecCount = 0, fileCount = 0;
    if (options.isDirectory)
    {
        
        getFilesInDirectory(inputPaths, options.inputPath);
        numFiles = length(inputPaths);
        std::cout << numFiles << ": SAM/BAM Files found under the directory: "
        << options.inputPath <<"! \n" ;
    }
    else
    {
        if (is_file(toCString(options.inputPath)))
            inputPaths.push_back(options.inputPath);
//           appendValue(inputPaths, options.inputPath);
        else
        {
            std::cout << options.inputPath
            << " is not a file use -d option for a directory.\n";
            return 1;
        }
    }
    

    std::stringstream ss;
    std::ofstream sam_extract_file;
    
    std::vector<std::string> uniqueReads;
    std::vector<std::string> uniqueReadsByTaxid;
    
    
    std::string nodes_path = options.mappingDir + "/nodes.dmp";
    std::string names_path = options.mappingDir + "/names.dmp";
    
    Timer<> MainStopWatch;
    // ============get the taxaID2name mapping ================
    std::cout<<"Loding taxaID2name mapping ... ";
    TIntStrMap taxaID2name;
    
    taxaID2name =  loadMappingInt2String<TIntStrMap, std::string>(names_path);
    std::cout<<"in " << MainStopWatch.lap() <<" secs [OK!]" << std::endl << std::endl;
    // ============get the node mapping ================
    std::cout<<"Loding node mapping ... " <<std::endl;
    TNodes nodes;
    loadNodes<>(nodes, nodes_path);
    
    std::cout<<"in " << MainStopWatch.lap() <<" secs [OK!]" << std::endl << std::endl;
    
  
    for(std::string currFile : inputPaths)
    {
        Timer<> PerFileStopWatch;
        fileCount ++;

        std::cout<<"================================================\nReading "
        <<fileCount<<" of "<< numFiles<<" files ...\n"
        <<getFilename(currFile)<<"\n";
        // read the original file
        BamFileIn bamFile;
        if (!open(bamFile, toCString(currFile)))
        {
            std::cerr << "Could not open " << currFile << "!\n";
            return 1;
        }
        
        Slimm slimm;
        slimm.options = options;
        BamHeader header;
        readHeader(header, bamFile);
        
        StringSet<CharString> refNames = contigNames(context(bamFile));
        StringSet<uint32_t> refLengths;
        refLengths = contigLengths(context(bamFile));
        
        slimm.references.resize(length(refNames));
        uint32_t noOfRefs = length(refNames);
        std::cout<<"computing features of each reference genome ... " << std::endl;
        
        // Determine taxa id position
        uint32_t tIdPos = 0;
        TIntIntMap gi2taxaID;
        bool giOnly = false;
        
        if (!getTaxaId(tIdPos, refNames[0], "ti"))
        {
            if (!getTaxaId(tIdPos, refNames[0], "kraken:taxid"))
            {

                std::cout<<"Unable to find a way to resolve taxon id associated with references.\n"
                <<"Make sure you used a set of references provided with SLIMM\n"
                <<"or generated by the preprocessing script.\n";
                return 1;
            }
        }

        if (slimm.options.binWidth == 0) //if binWidth is not given use avg read length
        {
            BamAlignmentRecord record;
            uint32_t count = 0, totlaLength = 0;
            while (!atEnd(bamFile) && count < 1000)
            {
                readRecord(record, bamFile);
                totlaLength += length(record.seq);
                ++count;
            }
            slimm.options.binWidth = totlaLength/count;
        }

        //reset the bamFile to the first recored by closing and reopening
        close(bamFile);

        open(bamFile, toCString(currFile));
        readHeader(header, bamFile);

        for (uint32_t i=0; i<noOfRefs; ++i)
        {
            ReferenceContig current_ref;
            current_ref.refName = refNames[i];
            current_ref.length = refLengths[i];
            
            // Intialize coverages based on the length of a refSeq
            Coverage cov(current_ref.length, slimm.options.binWidth);
            current_ref.cov = cov;
            current_ref.uniqCov = cov;
            current_ref.uniqCov2 = cov;
            StringList chunks;
            strSplit(chunks, refNames[i], EqualsChar<'|'>());
            current_ref.taxaID = giOnly?gi2taxaID[atoi(toCString(chunks[tIdPos]))]:atoi(toCString(chunks[tIdPos]));
            
            slimm.matchedTaxa.push_back(current_ref.taxaID);
            slimm.references[i] = current_ref;
        }
        std::cout<<"in " << PerFileStopWatch.lap() <<" secs [OK!]"  << std::endl << std::endl;
        
        
        std::cout<<"Analysing alignments, reads and references ..."<< std::endl;
        
        analyzeAlignments(slimm, bamFile);
        if (slimm.hitCount > 0)
        {
            std::cout << "  " <<  slimm.hitCount << " records processed." << std::endl;
            std::cout << "    " << slimm.noOfMatched << " matching reads" << std::endl;
            std::cout << "    " << slimm.noOfUniqlyMatched << " uniquily matching reads"<< std::endl;
            
            totalRecCount += slimm.noOfMatched;
            std::cout<<"in " << PerFileStopWatch.lap() <<" secs " << std::endl << std::endl;
            
            
            // Set the minimum reads to 10k-th of the total number of matched reads if not set by the user
            if (!isSet(parser, "min-reads"))
                slimm.options.minReads = 1 + ((slimm.noOfMatched - 1) / 10000);
            
            
            std::cout << "Number of Ref with reads = " << slimm.noOfRefs << std::endl;
            std::cout << "Expected Coverage = " << slimm.expCov() <<std::endl;
            std::cout << "Coverage Cutoff = " << slimm.covCutoff()
            << " (" << slimm.options.covCutOff <<" quantile)"<< std::endl;
            std::cout << "UniqCoverage Cutoff = " << slimm.uniqCovCutoff()
            << " (" << slimm.options.covCutOff <<" quantile)"<< std::endl;        
            
            
            std::cout   << "Filtering unlikely sequences ..."  << std::endl ;
            
            filterAlignments(slimm);
            
            std::cout << "  " << length(slimm.validRefTaxonIDs)
            << " passed the threshould coverage."<< std::endl;
            std::cout << "  " << slimm.failedByCov << " ref's couldn't pass the coverage threshould." << std::endl;
            std::cout << "  " << slimm.failedByUniqCov << " ref's couldn't pass the uniq coverage threshould." << std::endl;
            std::cout << "  Uniquily matching reads increased from "
            << slimm.noOfUniqlyMatched << " to "
            << slimm.noOfUniqlyMatched2 <<std::endl;
            std::cout << "in " << PerFileStopWatch.lap() <<" secs [OK!]"  << std::endl << std::endl;
        }
        else
        {
            std::cout << "[WARNING] No mapped reads found in BAM file!" << std::endl;
        }

        if (slimm.options.outputRaw)
        {
            std::cout<<"Writing features to a file ..." << std::endl;
            std::string tsvFile = getTSVFileName(options.outputPrefix, currFile);
            writeToFile(tsvFile, slimm.references, taxaID2name);
            std::cout<<"in " << PerFileStopWatch.lap() <<" secs [OK!]"  << std::endl << std::endl;
        }

        std::cout<<"Assigning reads to Least Common Ancestor (LCA)" << std::endl;
        getReadLCACount(slimm, nodes);
        std::cout<<"in " << PerFileStopWatch.lap() <<" secs [OK!]"  << std::endl << std::endl;
        
        std::cout<<"Writing taxnomic profile(s) ..." << std::endl;
        writeAbundance(slimm, nodes, taxaID2name, currFile);
        std::cout<<"in " << PerFileStopWatch.lap() <<" secs [OK!]"  << std::endl << std::endl;
        
        std::cout<<"File took " << PerFileStopWatch.elapsed()
        <<" secs to process." << std::endl;
    }

    std::string output_directory = getDirectory(options.outputPrefix);
    
    std::cout << "================================================" << std::endl
    << "================================================" << std::endl << std::endl;
    std::cout << totalRecCount
    << " SAM/BAM alignment records are proccessed."<<std::endl;
    std::cout << "extracted features are written to: "
    << output_directory <<std::endl;
    std::cout << "Total tame elapsed: "
    << MainStopWatch.elapsed() <<" secs"<<std::endl;

    return 0;
}
