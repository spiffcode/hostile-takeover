all: palm/sizeops.inc sizeops.h

palm/sizeops.inc: palm/tbltcode.s palm/tbltcodeleft.s palm/tbltcoderight.s palm/tbltcodey.s
	m68k-palmos-as -as --defsym SIZEOPS=0 palm/tbltcode.s > palm/tbltcode.sym
	m68k-palmos-as -as --defsym SIZEOPS=0 palm/tbltcodeleft.s > palm/tbltcodeleft.sym	
	m68k-palmos-as -as --defsym SIZEOPS=0 palm/tbltcoderight.s > palm/tbltcoderight.sym	
	m68k-palmos-as -as --defsym SIZEOPS=0 palm/tbltcodey.s > palm/tbltcodey.sym		
	../bin/sizeops -inc palm/tbltcode.sym palm/tbltcodeleft.sym palm/tbltcoderight.sym palm/tbltcodey.sym > palm/sizeops.inc
	../bin/sizeops -h palm/tbltcode.sym palm/tbltcodeleft.sym palm/tbltcoderight.sym palm/tbltcodey.sym > sizeops.h
	cp palm/sizeops.inc ../data/sizeops.inc
