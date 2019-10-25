
// compressor utility for Commodore Plus/4 programs
// Copyright (C) 2007-2017 Istvan Varga <istvanv@users.sourceforge.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// The Plus/4 program files generated by this utility are not covered by the
// GNU General Public License, and can be used, modified, and distributed
// without any restrictions.

#ifndef P4COMPRESS_COMPRESS0_HPP
#define P4COMPRESS_COMPRESS0_HPP

#include "plus4emu.hpp"
#include "compress.hpp"
#include "comprlib.hpp"

#include <vector>

namespace Plus4Compress {

  class Compressor_M0 : public Compressor {
   public:
    struct CompressionParameters {
      size_t  optimizeIterations;
      size_t  splitOptimizationDepth;
      CompressionParameters();
      CompressionParameters(const CompressionParameters& r);
      ~CompressionParameters()
      {
      }
      CompressionParameters& operator=(const CompressionParameters& r);
      void setCompressionLevel(int n);
    };
   private:
    static const size_t minRepeatDist = 1;
    static const size_t maxRepeatDist = 65536;
    static const size_t minRepeatLen = 2;
    static const size_t maxRepeatLen = 256;
   protected:
    class DSearchTable : public LZSearchTable {
     private:
      std::vector< std::vector< unsigned char > >   seqDiffTable;
      std::vector< std::vector< unsigned short > >  maxSeqLenTable;
     public:
      DSearchTable(size_t minLength, size_t maxLength, size_t maxOffs);
      virtual ~DSearchTable();
      void findMatches(const unsigned char *buf, size_t bufSize);
      inline size_t getSequenceLength(size_t bufPos, size_t d) const
      {
        return size_t(maxSeqLenTable[d - Compressor_M0::minRepeatDist][bufPos]);
      }
      inline unsigned char getSequenceDeltaValue(size_t bufPos, size_t d) const
      {
        return seqDiffTable[d - Compressor_M0::minRepeatDist][bufPos];
      }
    };
   private:
    struct LZMatchParameters {
      unsigned int    d;
      unsigned short  len;
      unsigned char   seqDiff;
      LZMatchParameters()
        : d(0),
          len(1),
          seqDiff(0x00)
      {
      }
      LZMatchParameters(const LZMatchParameters& r)
        : d(r.d),
          len(r.len),
          seqDiff(r.seqDiff)
      {
      }
      ~LZMatchParameters()
      {
      }
      inline LZMatchParameters& operator=(const LZMatchParameters& r)
      {
        d = r.d;
        len = r.len;
        seqDiff = r.seqDiff;
        return (*this);
      }
      inline void clear()
      {
        d = 0;
        len = 1;
        seqDiff = 0x00;
      }
    };
    // --------
    struct BitCountTableEntry {
      long    totalBits;
      unsigned int  prvDistances[4];
    };
   protected:
    struct SplitOptimizationBlock {
      std::vector< unsigned int > buf;
      size_t  startPos;
      size_t  nBytes;
      size_t  compressedSize;
      bool    isLastBlock;
    };
    // --------
    CompressionParameters config;
    unsigned short  *lengthCodeTable;
    unsigned char   *lengthBitsTable;
    unsigned int    *lengthValueTable;
    unsigned short  *distanceCodeTable;
    unsigned char   *distanceBitsTable;
    unsigned int    *distanceValueTable;
    size_t          *tmpCharBitsTable;
    DSearchTable    *searchTable;
    size_t          prvDistances[4];
    unsigned char   outputShiftReg;
    int             outputBitCnt;
    unsigned int    lfsrState;
    // for literals and distance codes
    HuffmanEncoder  huffmanEncoder1;
    // for length codes
    HuffmanEncoder  huffmanEncoder2;
    unsigned int    *symbolCntTable1;
    unsigned int    *symbolCntTable2;
    unsigned int    *encodeTable1;
    unsigned int    *encodeTable2;
    // --------
    static void huffmanCompatibilityHack(unsigned int *encodeTable,
                                         const unsigned int *symbolCnts,
                                         size_t nSymbols, bool reverseBits);
   private:
    void calculateHuffmanEncoding(std::vector< unsigned int >& ioBuf);
   protected:
    virtual void huffmanEncodeBlock(std::vector< unsigned int >& ioBuf,
                                    const unsigned char *inBuf,
                                    size_t uncompressedBytes);
   private:
    void initializeLengthCodeTables();
    PLUS4EMU_INLINE void encodeSymbol(std::vector< unsigned int >& buf,
                                      unsigned int c);
    void writeRepeatCode(std::vector< unsigned int >& buf, size_t d, size_t n);
    void writeSequenceCode(std::vector< unsigned int >& buf,
                           unsigned char seqDiff, size_t d, size_t n);
   protected:
    PLUS4EMU_INLINE long rndBit();
   private:
    void optimizeMatches_RND(
        LZMatchParameters *matchTable, BitCountTableEntry *bitCountTable,
        const size_t *lengthBitsTable_, const unsigned char *inBuf,
        size_t offs, size_t nBytes);
    void optimizeMatches(
        LZMatchParameters *matchTable, BitCountTableEntry *bitCountTable,
        const size_t *lengthBitsTable_, const unsigned char *inBuf,
        size_t offs, size_t nBytes);
    void compressData_(std::vector< unsigned int >& tmpOutBuf,
                       const std::vector< unsigned char >& inBuf,
                       size_t offs, size_t nBytes);
   protected:
    virtual bool compressData(std::vector< unsigned int >& tmpOutBuf,
                              const std::vector< unsigned char >& inBuf,
                              unsigned int startAddr, bool isLastBlock,
                              size_t offs = 0, size_t nBytes = 0x7FFFFFFFUL);
   private:
    bool compressBlock(SplitOptimizationBlock& tmpBlock,
                       const std::vector< unsigned char >& inBuf,
                       unsigned int startAddr, size_t startPos, size_t nBytes,
                       bool isLastBlock);
   protected:
    virtual void packOutputData(const std::vector< unsigned int >& tmpBuf,
                                bool isLastBlock);
    Compressor_M0(std::vector< unsigned char >& outBuf_,
                  size_t huff1Size, size_t huff1MinCnt,
                  size_t huff2Size, size_t huff2MinCnt);
   public:
    Compressor_M0(std::vector< unsigned char >& outBuf_);
    virtual ~Compressor_M0();
    void getCompressionParameters(CompressionParameters& cfg) const;
    void setCompressionParameters(const CompressionParameters& cfg);
    virtual void setCompressionLevel(int n);
    virtual void addZeroPageUpdate(unsigned int endAddr, bool isLastBlock);
    // if 'startAddr' is 0xFFFFFFFF, it is not stored in the compressed data
    virtual bool compressData(const std::vector< unsigned char >& inBuf,
                              unsigned int startAddr, bool isLastBlock,
                              bool enableProgressDisplay = false);
  };

}       // namespace Plus4Compress

#endif  // P4COMPRESS_COMPRESS0_HPP

