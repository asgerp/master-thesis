from __future__ import print_function
import os, random, sys




def create_menu(num, fnames ):
    f = open('menus/menu'+str(num)+'.tex','w+')
    latex = "\documentclass[10pt, conference, compsocconf]{IEEEtran}\n\usepackage[utf8]{inputenc}\n\usepackage{array}\n\usepackage{mdwmath}\n\usepackage{mdwtab}\n\usepackage[caption=false,font=footnotesize]{subfig}\n\usepackage{graphicx}\n"
    latex += "\\begin{document}\n"
    if(num == 4):
        latex += "\\begin{figure*}[t]\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[0] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[1] + "}}\hfill\n"
        latex += '\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/white.png}}\\\\ \n'
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[2] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[3] + "}}\hfill"
        latex += '\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/white.png}}\\\\ \n'
        latex += "\end{figure*}\n"
    if(num == 8):
        latex += "\\begin{figure*}[t]\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[0] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[1] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[2] + "}}\\\\ \n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[3] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/white.png}}\hfill\n"
        latex += '\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/' + fnames[4] + '}}\\\\\n '
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[5] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[6] + "}}\hfill\n"
        latex += '\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/' + fnames[7] + '}}\\\\ \n'
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/white.png}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/white.png}}\hfill\n"
        latex += '\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/white.png}}\\\\ \n'
        latex += "\end{figure*}\n"
    if(num == 12):
        latex += "\\begin{figure*}[t]\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[0] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[1] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[2] + "}}\\\\ \n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[3] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[4] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[5] + "}}\\\\ \n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[6] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[7] + "}}\hfill\n"
        latex += '\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/' + fnames[8] + '}}\\\\ \n'
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[9] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[10] + "}}\hfill\n"
        latex += "\subfloat{\includegraphics[width=0.3\\textwidth]{menus/icons/" + fnames[11] + "}}\\\\ \n"
        latex += "\end{figure*}\n"
    latex += "\end{document}\n"
    print(latex, file=f)
    

def make_file(file_name):
    os.system("latexmk -pdf -f -silent menus/" + file_name + ".tex")
    os.rename(file_name + ".pdf", "menus/" + file_name + ".pdf")
    os.remove(file_name + ".aux")
    os.remove(file_name + ".fdb_latexmk")
    os.remove(file_name + ".fls")
    os.remove(file_name + ".log")
    os.system("lpr menus/" + file_name + ".pdf")
    