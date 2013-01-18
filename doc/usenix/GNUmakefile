MAIN = fvm.ltx

BASE      := $(basename $(MAIN))
SRCEXT    := $(patsubst $(BASE).%,%,$(MAIN))
SVGFIGS   := $(wildcard fig/*.svg)
XFIGS     := $(wildcard fig/*.fig)
DIAFIGS   := $(wildcard fig/*.dia)
TEXFIGS   := $(patsubst fig/%.fig, fig/%.tex, $(XFIGS)) # $(DIAFIGS:.dia=.tex)
BINFIGS   := $(patsubst fig/%.fig, fig/%.pdf, $(XFIGS)) \
    	     $(patsubst fig/%.dia, fig/%.pdf, $(DIAFIGS)) \
    	     $(patsubst fig/%.svg, fig/%.pdf, $(SVGFIGS)) 
STYLINK	  := $(notdir $(wildcard ../*.sty))
STYFILES  := $(sort $(STYLINK) $(wildcard *.sty))
TEXFILES  := $(sort $(MAIN) $(wildcard *.tex) $(STYFILES) $(TEXFIGS))

BIBFILES  := $(wildcard *.bib)
#GENBIBFILES := mikerefs/short.bib princembib/short.bib
GENBIBFILES := 
ALLBIBFILES := $(BIBFILES) $(GENBIBFILES)
BIBDIST   := $(filter-out $(GENBIBFILES), $(BIBFILES))

JGRAPHS   := $(wildcard jgraph/*.j)
GGRAPHS   := $(wildcard gp/*.gp)
BARGRAPHS := $(wildcard gp/*.sh)
BAREPS    := $(patsubst gp/%.sh, graphs/%.eps, $(BARGRAPHS))
BARGRAPHDRIVER := $(patsubst %, gp/%, barchart barchart-stacked)

GRAPHS    := $(patsubst jgraph/%.j, graphs/%.pdf, $(JGRAPHS)) \
    	     $(patsubst gp/%.gp, graphs/%.pdf, $(filter-out gp/style.gp, $(GGRAPHS))) \
	     $(patsubst gp/%.sh, graphs/%.pdf, $(BARGRAPHS))

DATA      := $(wildcard dat/*.dat)
LOGREPORTS := $(wildcard log_reports/*)
PROCREPORTS := $(wildcard proc_reports/*)   

GENTEX := 

# "source" for image file PDFs that we will generate
IMAGESRC  := $(wildcard figures/*.gif) $(wildcard figures/*.eps) 
# the name of those generated PDFs
#IMAGEGEN  := $(addsuffix .pdf, $(basename $(IMAGESRC)))
# combine with pre-generated PDF. the pre-generatd PDF is if we manually go from
# .{ppt,graffle,odp} to PDF using those programs' export features, for
# instance
IMAGES    := $(sort $(IMAGEGEN) $(wildcard figures/*.pdf rough-figs/*.pdf))
# but don't distribute generated files
IMAGEDIST := $(filter-out $(IMAGEGEN), $(IMAGES)) \
		$(wildcard figures/*.odp) $(wildcard figures/*.ppt) \
		$(wildcard figures/*.graffle)

# LATEX \def\noeditingmarks{} \input
LATEX     = pdflatex #\\nonstopmode\\input
#LATEX     = pdflatex \\def\\dlaTR{} \\input
BIBTEX    = bibtex -min-crossrefs=1000
PS2PDF    = ps2pdf

SHA = shasum

all: $(BASE).pdf 

%/short.bib: %/generic.bib %/shortenBib.awk
	rm -f $@
	awk -f $*/shortenBib.awk < $< > $@
	chmod a-w $@

data-stamp: $(LOGREPORTS) $(PROCREPORTS)
	for i in `ls proc_reports/*.pl proc_reports/*.sh`; do $$i; done
	touch $@

# to include an xfig with embedded latex, you include the *tex* file:
# \begin{figure}
#    \input{foo}
# \end{figure}
#  where fig/foo.fig is an xfig file that you created with embedded latex.
%.pdf %.tex: %.fig
	fig2dev -L pdftex -p1 $< > $*.pdf
	fig2dev -L pdftex_t -p $*.pdf $< > $*.tex

%.pdf: %.eps
	perl -ne 'exit 1 unless /\n/' $< \
		|| perl -p -i -e 's/\r/\n/g' $<
	epstopdf --outfile=$@ $<

%.pdf: %.gif
	convert $< $@

%.tex: %.dia
	unset DISPLAY; dia -n -e $@ $<

# dirt-simple rule to build graphs. depends on gnuplot file printing its
# output (the PDF file) to stdout
graphs/%.pdf: gp/%.gp data-stamp 
	gnuplot $< > $@

graphs/%.pdf: jgraph/%.j data-stamp 
	@rm -f $@~
	jgraph $< > $@~
	mv -f $@~ $@

graphs/%.pdf: gp/%.sh $(BARGRAPHDRIVER) data-stamp
	@rm -f $@~
	$< 

#convert SVG figures to PDF
fig/%.pdf: fig/%.svg
	@rm -f $@~
	inkscape --export-pdf=$@~ $<
	mv -f $@~ $@

$(STYLINK):
	rm -f $@
	ln -s ../$@ .

RERUN = egrep -q '(^LaTeX Warning:|\(natbib\)).* Rerun' $*.log
UNDEFINED = egrep -q '^(LaTeX|Package natbib) Warning:.* undefined' $*.log

# So you can "setenv ALWAYS yes" instead of butchering the Makefile...
ifneq ($(ALWAYS),)
ALWAYS = always
endif
##$(GRAPHS) \
#				$(GGRAPHS) $(JGRAPHS) $(BARGRAPHS)\
#				$(BARGRAPHDRIVER)\
#				$(LOGREPORTS) \
#				$(PROCREPORTS) \
#				data-stamp
$(BASE).pdf: %.pdf: %.$(SRCEXT) $(TEXFILES) $(ALLBIBFILES) \
				$(BINFIGS) $(GENTEX) $(TEXFIGS)	\
				$(IMAGES) $(ALWAYS)
	test ! -s $*.aux || $(BIBTEX) $* || rm -f $*.aux $*.bbl
	$(LATEX) $< || ! rm -f $@
	if $(UNDEFINED); then \
		$(BIBTEX) $* && $(LATEX) $< || ! rm -f $*.bbl $@; \
	fi
	! $(RERUN) || $(LATEX) $< || ! rm -f $@
	! $(RERUN) || $(LATEX) $< || ! rm -f $@
	touch $*.dvi
	test ! -f .xpdf-running || xpdf -remote $(BASE)-server -reload

%.pdf: %.ltx
	$(LATEX) $< || !rm -f $@
	! $(RERUN) || $(LATEX) $< || ! rm -f $@
	! $(RERUN) || $(LATEX) $< || ! rm -f $@

DIST = GNUmakefile $(filter-out $(GENTEX) $(TEXFIGS), $(TEXFILES)) \
	$(XFIGS) $(DIAFIGS) \
	$(JGRAPHS) $(GGRAPHS) $(BARGRAPHS) \
	$(BARGRAPHDRIVER) \
	$(LOGREPORTS) $(PROCREPORTS) \
	$(IMAGESRC) $(IMAGEDIST) $(BIBDIST)
dist: $(BASE).tar.gz
$(BASE).tar.gz: $(DIST)
	tar -chzf $@ -C .. \
		$(patsubst %, $(notdir $(PWD))/%, $(DIST))

PREVIEW := $(BASE)
preview: $(PREVIEW).pdf
	if test -f .xpdf-running; then			\
		xpdf -remote $(BASE)-server -quit;	\
		sleep 1;				\
	fi
	touch .xpdf-running
	(xpdf -remote $(BASE)-server $(PREVIEW).pdf; rm -f .xpdf-running) &

show: $(BASE).pdf
	xpdf -fullscreen $(BASE).pdf

osx: $(BASE).pdf
	open $(BASE).pdf

anon: $(BASE)-anon.pdf

double:
	for i in $(TEXFILES); do perl double.pl $$i; done

# stupid hack to anonymize a PDF. it's maybe ridiculous that this
# works.
$(BASE)-anon.pdf: $(BASE).pdf
	$(PS2PDF) $< $@

EXTRAIGNORE = '*~' '*.aux' '*.bbl' '*.blg' '*.log' '*.dvi' '*.bak' '*.out'
ignore:
	rm -f .gitignore
	(for file in $(EXTRAIGNORE); do \
		echo "$$file"; \
	done; \
	for file in .xpdf-running \
			$(BASE).tar.gz \
			$(GENTEX) $(TEXFIGS) $(BINFIGS) $(GRAPHS) $(IMAGEGEN) \
			$(BASE).ps $(BASE).pdf; do \
		echo "/$$file"; \
	done) | sort > .gitignore
	git add .gitignore

cleangraphs:
	rm -f $(GRAPHS)

clean:
	rm -f $(BASE).ps $(BASE).pdf $(BASE).tar.gz
#	rm -f $(TEXFIGS) $(BINFIGS) $(GRAPHS) $(IMAGEGEN)
	rm -f $(GENTEX)
	rm -f $(DATA)
	rm -f *.dvi *.aux *.log *.bbl *.blg *.lof *.lot *.toc *.bak *.out
	rm -f *~ *.core core
	rm -f $(BAREPS)
	rm -f data-stamp

always:
	@:

.PHONY: install clean all always ignore preview show osx dist

spell:
	make clean
	for i in $(wildcard *.tex); do aspell -p ./aspell.words -c $$i; done
