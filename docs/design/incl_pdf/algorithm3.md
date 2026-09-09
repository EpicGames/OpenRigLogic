\begin{algorithm}
\DontPrintSemicolon
\KwIn{Input indices $InInd[]$}
\NextIn{Output indices $OutInd[C]$}
\NextIn{Values $Vals[]$}
\NextIn{Input vector $InVec[]$}
\NextIn{Output vector $OutVec[]$}
\KwOut{Modifies $OutVec[]$ inplace}
$offset \gets 0$\;
\For{$c \gets 0$ \textbf{to} $C - 1$}{
  $i \gets InInd_c$\;
  $value \gets InVec_i$\;
  $from \gets Vals[offset + FROM\_INDEX]$\;
  $to \gets Vals[offset + TO\_INDEX]$\;
  \If{from < value < to}{
    $s \gets Vals[offset + SLOPE\_INDEX]$\;
    $c \gets Vals[offset + CUT\_INDEX]$\;
    $t \gets OutInd_c$\;
    $OutVec_{t} \gets OutVec_{t} + (s * value + c)$\;
  }
  $offset \gets offset + COL\_SIZE$\;
}
\caption{{\sc EvalCond} Evaluate Conditional Dependent Outputs - See Appendix A}
\label{algo:EvalCond}
\end{algorithm}