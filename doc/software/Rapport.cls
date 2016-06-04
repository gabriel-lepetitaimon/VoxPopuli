\LoadClass{report} 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%		GEOMETRY				%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\RequirePackage{geometry}
\geometry{
 	a4paper,
 	total={150mm,240mm},
 	left=30mm,
 	top=30mm
 }

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%		   	 TITLES				%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\RequirePackage{titlesec}

\titleformat{\section} 
{\Large\scshape\raggedright} % Make the \section headers large (\Large),
{}{0em}                      % Can be used to give a prefix to all sections, like 'Section ...'
{}                           % Can be used to insert code before the heading
[]



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%		HEADERS & FOOTERS		%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

\RequirePackage{fancyhdr}

\pagestyle{fancy}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%			FONTS				%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\RequirePackage[T1]{fontenc}
\RequirePackage{lmodern}

