
DOTSRC			:= $(wildcard gv/*.dot)
DOTOBJ			:= $(patsubst gv/%.dot, gv/%.dot.pdf, $(DOTSRC))
FDPSRC			:= $(wildcard gv/*.fdp)
FDPOBJ			:= $(patsubst gv/%.fdp, gv/%.fdp.pdf, $(FDPSRC))

all: $(DOTOBJ) $(FDPOBJ)

gv/%.dot.pdf: gv/%.dot
	dot -Tpdf $< > $@

gv/%.fdp.pdf: gv/%.fdp
	fdp -Tpdf $< > $@

clean:
	rm -f $(DOTOBJ)

