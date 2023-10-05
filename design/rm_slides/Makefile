SOURCE=preamble.tex
TARGET=slides
FLAGS=-halt-on-error -file-line-error -c-style-errors
SVGs := $(patsubst %.svg,%.pdf, $(wildcard *.svg))
#PNGs := $(patsubst %.png,%.eps, $(wildcard *.png))
#JPGs := $(patsubst %.jpg,%.eps, $(wildcard *.jpg))
#FIGs := $(patsubst %.fig,%.eps, $(wildcard *.fig))
TEXs := $(wildcard *.tex)
.PHONY =dvi ps pdf mpost clean cleanall

$(TARGET).pdf: $(TEXs) $(SVGs)
	pdflatex $(FLAGS) -jobname $(TARGET) $(SOURCE)
	pdflatex $(FLAGS) -jobname $(TARGET) $(SOURCE)

clean:
	rm -f \#*
	rm -f *~
	rm -f *.log
	rm -f *.toc
	rm -f *.aux
	rm -f $(TARGET).dvi
	rm -f $(TARGET).ps
	rm -f $(TARGET).pdf
	rm -f *.eps
	rm -f $(TARGET).nav $(TARGET).out $(TARGET).snm	
	rm -f $(SVGs)

%.eps: %.jpg
	convert $< $@

%.eps: %.png
	convert $< $@

%.eps: %.fig
	fig2dev -L eps $< $@

%.pdf: %.svg
	inkscape $< -B -A $@
