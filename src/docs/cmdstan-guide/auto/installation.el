(TeX-add-style-hook
 "installation"
 (lambda ()
   (LaTeX-add-labels
    "install.appendix"
    "install-windows.appendix"
    "install-mac.appendix"
    "install-linux.appendix"
    "make-options.appendix"))
 :latex)

