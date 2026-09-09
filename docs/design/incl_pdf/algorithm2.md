\begin{algorithm}
\DontPrintSemicolon
\KwIn{Linear matrix non-zero values $Mat[]$}
\NextIn{Non-zero count per row $Nz[R]$}
\NextIn{Column indices of non-zeros $Cols[]$}
\NextIn{Linear Indices $LinInd[]$}
\NextIn{Input vector $InVec[]$}
\NextIn{Output vector $OutVec[]$}
\KwOut{Modifies $OutVec[]$ inplace}
$offset \gets 0$\;
\For{$r \gets 0$ \textbf{to} $R - 1$} {
  $out \gets 0.0$\;
  \For{$n \gets 0$ \textbf{to} $Nz_r - 1$} {
    $i \gets offset + n$\;
    $col \gets Cols_i$\;
    $out \gets out + Mat_i * InVec_{col}$\;
  }
  $l \gets LinInd_r$\;
  $OutVec_{l} \gets OutVec_{l} + out$\;
  $offset \gets offset + Nz_r$\;
}
\caption{{\sc EvalLinear} Evaluate Linear Outputs - See Appendix A}
\label{algo:EvalLinear}
\end{algorithm}