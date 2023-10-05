SOURCE=preamble.tex
TARGET=designdokument
FLAGS=-halt-on-error -file-line-error -c-style-errors
#PNGs := $(patsubst %.png,%.eps, $(wildcard *.png))
JPGs := $(patsubst %.jpg,%.eps, $(wildcard *.jpg))
FIGs := $(patsubst %.fig,%.eps, $(wildcard *.fig))
TEXs := *.tex #$(patsubst %.fig,%.eps, $(wildcard *.fig))
.PHONY =dvi ps pdf mpost clean cleanall

all: dvi ps pdf

dvi: $(TARGET).dvi
ps: $(TARGET).ps
pdf: $(TARGET).pdf

$(TARGET).dvi: $(JPGs) $(FIGs) $(TEXs)
	latex $(FLAGS) -jobname $(TARGET) $(SOURCE)
	latex $(FLAGS) -jobname $(TARGET) $(SOURCE)

$(TARGET).ps: $(TARGET).dvi
	dvips -t a4 $(TARGET).dvi -o

$(TARGET).pdf: $(TARGET).ps
	ps2pdf14 $(TARGET).ps $(TARGET).pdf

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

%.eps: %.jpg
	convert $< $@

%.eps: %.png
	convert $< $@

%.eps: %.fig
	fig2dev -L eps $< $@
