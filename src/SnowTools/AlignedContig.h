#ifndef ALIGNED_CONTIG_H
#define ALIGNED_CONTIG_H

#include <algorithm>

#include "SnowTools/BamRead.h"
#include "SnowTools/BreakPoint.h"
#include "SnowTools/DiscordantCluster.h"
#include "SnowTools/BWAWrapper.h"
#include "SnowTools/BamWalker.h"

namespace SnowTools {

  class AlignedContig;
  typedef std::unordered_map<std::string, AlignedContig> ContigMap;
  typedef std::vector<AlignedContig> AlignedContigVec;
  
  /*! This class contains a single alignment fragment from a contig to
   * the reference. For a multi-part mapping of a contig to the reference,
   * an object of this class represents just a single fragment from that alignment.
   */
  struct AlignmentFragment {
    
    friend AlignedContig;
    
    /*! Construct an AlignmentFragment from a BWA alignment
     * @param const reference to an aligned sequencing read
     */
    AlignmentFragment(const BamRead &talign);
    
    //! sort AlignmentFragment objects by start position
    bool operator < (const AlignmentFragment& str) const { return (start < str.start); }
    
    //! print the AlignmentFragment
    friend std::ostream& operator<<(std::ostream &out, const AlignmentFragment& c); 
    
    /*! @function
     * @abstract Parse an alignment frag for a breakpoint
     * @param reference to a BreakPoint to be created
     * @return boolean informing whether there was a remaining indel break
     */
    bool parseIndelBreak(BreakPoint &bp);
    
    /*! @function check if breakpoints match any cigar strings direct from the reads
     * This is important in case the assembly missed a read (esp normal) that indicates
     * that there is an indel. Basically this function says that if assembly calls a breakpoint
     * and it agrees with a read alignment breakpoint, combine the info.
     * @param const CigarMap reference containing hash with key=chr_breakpos_indeltype, val=tumor count
     * @param const CigarMap reference containing hash with key=chr_breakpos_indeltype, val=normal count
     */
    void indelCigarMatches(const CigarMap &nmap, const CigarMap &tmap);  

    /** Check whether the alignment fragement overlaps with the given windows
     * 
     * This function is used to 
     */
    bool checkLocal(const GenomicRegion& window);
    
    const BPVec& getIndelBreaks() const { return m_indel_breaks; }
    
    /*! Write the alignment record to a BAM file
     */
    void writeToBAM(BamWalker& bw) { 
      bw.WriteAlignment(m_align); 
    } 


    private:
    
    BPVec m_indel_breaks; /**< indel variants on this alignment */
    
    Cigar m_cigar; /**< cigar oriented to assembled orientation */
    
    BamRead m_align; /**< BWA alignment to reference */
    
    size_t idx = 0; // index of the cigar where the last indel was taken from 
    
    int break1 = -1; // 0-based breakpoint 1 on contig 
    int break2 = -1; /**< 0-based breakpoint 2 on contig */
    int gbreak1 = -1; /**< 0-based breakpoint 1 on reference chr */
    int gbreak2 = -1; /**< 0-based breakpoint 1 on reference chr */
    
    int start; /**< the start position of this alignment on the reference. */
    
    bool local = false; /**< boolean to note whether this fragment aligns to same location is was assembled from */
    
    AlignedContig * c; // link to the parent aligned contigs

    int num_align = 0;
  };
  
  //! vector of AlignmentFragment objects
  typedef std::vector<AlignmentFragment> AlignmentFragmentVector;
  
  /*! Contains the mapping of an aligned contig to the reference genome,
   * along with pointer to all of the reads aligned to this contig, and a 
   * store of all of the breakpoints associated with this contig
   */
  class AlignedContig {
    
    friend class AlignmentFragment;
    
  public:  
    
    AlignedContig() {}
    
    AlignedContig(const BamReadVector& bav);
    
    /*! Constructor which parses an alignment record from BWA (a potentially multi-line SAM record)
     * @param const reference to a string representing a SAM alignment (contains newlines if multi-part alignment)
     * @param const reference to a SnowTools::GenomicRegion window specifying where in the reference this contig was assembled from.
     */
    //AlignedContig(const std::string &sam, const SnowTools::GenomicRegion &twindow, 
    //		const CigarMap &nmap, const CigarMap &tmap);
    
    //std::string samrecord; /**< the original SAM record */
    
    /*! @function Determine if this contig has identical breaks and is better than another.
     * @param const reference to another AlignedContig
     * @return bool bool returning true iff this contig has identical info has better MAPQ, or equal MAPQ but longer */
    bool isWorse(const AlignedContig &ac) const;

    /*! @function Loop through all the alignment framgents and their indel breaks and check against cigar database
     */
    void checkAgainstCigarMatches(const CigarMap& nmap, const CigarMap& tmap);
    
    /*! @function
      @abstract  Get whether the query is on the reverse strand
      @param  b  pointer to an alignment
      @return    boolean true if query is on the reverse strand
    */
    void addAlignment(const BamRead &align);
    
    //! Loop through fragments and check if they overlap with window (and set local flag). Return TRUE if local found
    bool checkLocal(const GenomicRegion& window);
    
    /*! @function loop through the vector of DiscordantCluster objects
     * associated with this contig and print
     */
    std::string printDiscordantClusters() const;
    
    //! return the name of the contig
    std::string getContigName() const { 
      if (!m_frag_v.size()) 
	return "";  
      return m_frag_v[0].m_align.Qname(); 
    }
    
    /*! @function get the maximum mapping quality from all alignments
     * @return int max mapq
     */
    int getMaxMapq() const { 
      int m = -1;
      for (auto& i : m_frag_v)
	if (i.m_align.MapQuality() > m)
	  m = i.m_align.MapQuality();
      return m;
      
    }
    
    /*! @function get the minimum mapping quality from all alignments
     * @return int min mapq
     */
    int getMinMapq() const { 
      int m = 1000;
      for (auto& i : m_frag_v)
	if (i.m_align.MapQuality() < m)
	  m = i.m_align.MapQuality();
      return m;
    }
    
    /*! @function loop through all of the breakpoints and
     * calculate the split read support for each. Requires 
     * alignedReads to have been run first (will error if not run).
     */
    void splitCoverage();
    
    /*! Checks if any of the indel breaks are in a blacklist. If so, mark the
     * breakpoints of the indels for skipping. That is, hasMinmal() will return false;
     * @param grv The blaclist regions
     */
    void blacklist(GRC& grv);
    
    /*! @function if this is a Discovar contig, extract
     * the tumor and normal read support
     * @param reference to int to fill for normal support
     * @param reference to int to fill for tumor support
     * @return boolean reporting if this is a discovar name
     */
    //bool parseDiscovarName(size_t &tumor, size_t &normal);
    
    /*! @function dump the contigs to a fasta
     * @param ostream to write to
     */
    void printContigFasta(std::ofstream &os) const;

  /*! @function set the breakpoints on the reference by combining multi-mapped contigs
   */
  void setMultiMapBreakPairs();

  /**
   */
  void alignReads(BamReadVector &bav);

  //! return the contig sequence as it came off the assembler
  std::string getSequence() const { assert(m_seq.length()); return m_seq; }

  //! detemine if the contig contains a subsequence
  bool hasSubSequence(const std::string& subseq) const { 
    return (m_seq.find(subseq) != std::string::npos);
  }
  
  //! print this contig
  friend std::ostream& operator<<(std::ostream &out, const AlignedContig &ac);

  /*! @function query if this contig contains a potential variant (indel or multi-map)
   * @return true if there is multimapping or an indel
   */
  bool hasVariant() const;

  /*! Write all of the alignment records to a BAM file
   * @param bw BamWalker opened with OpenWriteBam
   */
  void writeToBAM(BamWalker& bw) { 
    for (auto& i : m_frag_v) {
      //std::cerr << "write to bam  " << i << std::endl;
      i.writeToBAM(bw);
    }
  } 

  /*! Write all of the sequencing reads as aligned to contig to a BAM file
   * @param bw BamWalker opened with OpenWriteBam
   */
  void writeAlignedReadsToBAM(BamWalker& bw) { 
    for (auto& i : m_bamreads)
      bw.WriteAlignment(i);
  } 


  bool hasLocal() const { for (auto& i : m_frag_v) if (i.local) return true; return false; }

  /*! @function retrieves all of the breakpoints by combining indels with global mutli-map break
   * @return vector of ind
   */
  std::vector<BreakPoint> getAllBreakPoints() const;

  std::vector<const BreakPoint*> getAllBreakPointPointers() const ;

  void addDiscordantCluster(DiscordantClusterMap& dmap);
  
  int insertion_against_contig_read_count = 0;
  int deletion_against_contig_read_count = 0;

 private:

  BamReadVector m_bamreads; // store all of the reads aligned to contig

  std::vector<BreakPoint> m_local_breaks; // store all of the multi-map BreakPoints for this contigs 

  BreakPoint m_global_bp;  // store the single spanning BreakPoing for this contig e

  //ReadVec m_bamreads; // store smart pointers to all of the reads that align to this contig 

  bool m_skip = false; // flag to specify that we should minimally process and simply dump to contigs_all.sam 

  AlignmentFragmentVector m_frag_v; // store all of the individual alignment fragments 

  GenomicRegion m_window; /**< reference window from where this contig was assembled */

  bool m_hasvariant = false; // flag to specify whether this alignment has some potential varaint (eg indel)

  std::string m_seq = ""; // sequence of contig as it came off of assembler

  std::vector<DiscordantCluster> m_dc; // collection of all discordant clusters that map to same location as this contig

};

struct PlottedRead {

  int pos;
  std::string seq;
  std::string info;

  bool operator<(const PlottedRead& pr) const {
    return (pos < pr.pos);
  }

};

typedef std::vector<PlottedRead> PlottedReadVector;

struct PlottedReadLine {

  std::vector<PlottedRead*> read_vec;
  int available = 0;
  int contig_len = 0;

  void addRead(PlottedRead *r) {
    read_vec.push_back(r);
    available = r->pos + r->seq.length() + 5;
  }

  bool readFits(PlottedRead &r) {
    return (r.pos >= available);
  }

  friend std::ostream& operator<<(std::ostream& out, const PlottedReadLine &r) {
    int last_loc = 0;
    for (auto& i : r.read_vec) {
      assert(i->pos - last_loc >= 0);
      out << std::string(i->pos - last_loc, ' ') << i->seq;
      last_loc = i->pos + i->seq.length();
    }
    int name_buff = r.contig_len - last_loc;
    assert(name_buff < 10000);
    out << std::string(std::max(name_buff, 5), ' ');
    for (auto& i : r.read_vec) { // add the data
      out << i->info << ",";
    }
    return out;
  }

};

typedef std::vector<PlottedReadLine> PlottedReadLineVector;

}

#endif


