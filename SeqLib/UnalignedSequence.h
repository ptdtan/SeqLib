#ifndef SEQLIB_UNALIGNED_SEQ_H__
#define SEQLIB_UNALIGNED_SEQ_H__

#include <cstring>
#include <vector>

namespace SeqLib {

  /** Structure to hold unaligned sequence (name and bases)
   */
  struct UnalignedSequence {
  
    /** Construct an empty sequence */
    UnalignedSequence() {}
  
    /** Construct an unaliged sequence with name and sequence
     * @param n Name of the sequence 
     * @param s Sequence, stored as ACTG or N characters
     */
    UnalignedSequence(const std::string& n, const std::string& s) : Name(n), Seq(s), Qual(std::string()), Strand('*') {}

    /** Construct an unaliged sequence with name, sequence and quality score
     * @param n Name of the sequence 
     * @param s Sequence, stored as ACTG or N characters
     * @param q Quality string
     */
    UnalignedSequence(const std::string& n, const std::string& s, const std::string& q) : Name(n), Seq(s), Qual(q), Strand('*') {}

    /** Construct an unaliged sequence with name, sequence and quality score
     * @param n Name of the sequence 
     * @param s Sequence, stored as ACTG or N characters
     * @param q Quality string
     * @param t Strand of the sequence, one of '*', '+', '-'
     */
    UnalignedSequence(const std::string& n, const std::string& s, const std::string& q, char t) : Name(n), Seq(s), Qual(q), Strand(t) {}

     std::string Name; ///< Name of the contig
     std::string Seq;  ///< Sequence of the contig (upper-case ACTGN)
     std::string Qual; ///< Quality scores
     char Strand;      ///< Strand of the sequence. Default is '*'
  };

  typedef std::vector<UnalignedSequence> UnalignedSequenceVector; ///< A collection of unaligned sequences

}

#endif
