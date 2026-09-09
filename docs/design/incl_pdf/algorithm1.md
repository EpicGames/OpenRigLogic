\begin{algorithm}
\DontPrintSemicolon
\KwIn{PSD matrix non-zero values $Mat[]$}
\NextIn{Non-zero count per row $Nz[R]$}
\NextIn{Column indices of non-zeros $Cols[]$}
\NextIn{Input vector $InVec[]$}
\KwOut{Modifies $InVec[]$ inplace}
$offset \gets 0$\;
\For{$r \gets 0$ \textbf{to} $R - 1$} {
  $psd \gets 1.0$\;
  \For{$n \gets 0$ \textbf{to} $Nz_r - 1$} {
    $i \gets offset + n$\;
    $col \gets Cols_i$\;
    $prod \gets Mat_i * InVec_{col}$\;
    $psd \gets psd * min(PSD\_MAX, prod)$\;
  }
  $InVec_r \gets psd$\;
  $offset \gets offset + Nz_r$\;
}
\caption{{\sc EvalPsd} Evaluate PSD Inputs - See Appendix A}
\label{algo:EvalPsd}
\end{algorithm}