#!/bin/sh

datatool -pch ncbi_pch.hpp -m ../../objects/seqset/seqset.asn  -m ../../objects/general/general.asn -m ../../objects/seq/seq.asn  -m ../../objects/seqloc/seqloc.asn -m ../../objects/seqfeat/seqfeat.asn -m ../../objects/seqblock/seqblock.asn  -m ../../objects/seqalign/seqalign.asn -m ../../objects/pub/pub.asn  -m ../../objects/biblio/biblio.asn  -m ../../objects/seqres/seqres.asn -m ../../objects/seqtable/seqtable.asn -m ../../objects/medline/medline.asn -m ../../objects/submit/submit.asn -oA -tvs autogenerated_cleanup.txt

