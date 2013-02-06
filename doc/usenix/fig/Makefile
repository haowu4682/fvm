
SVGFIGS = $(wildcard *.svg)
BINFIGS = $(patsubst %.svg, %.pdf, $(SVGFIGS))

all: $(BINFIGS)

%.pdf: %.svg
	@rm -f $@~
	inkscape --export-pdf=$@~ $<
	mv -f $@~ $@


