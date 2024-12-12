#!/bin/bash

# This script can be useful to modify the markdown source files of the docs used by mkdocs to a format that can be (almost) correctly formatted with xWiki in markdown mode. The main modifications are to replace the following :
# Inline math : \\( ...some math... \\) with the mathjax macro {{mathjax}} ...some math... {{/mathjax}}
# Display mode math : $$ ...some math... $$ with the mathjax macro and an equation environement {{mathjax}}\begin{equation*} ...some math... \end{equation*}{{/mathjax}}
# Special characters that are managed differently within mkdocs and xWiki

# copy reference guide md files and assets
rm -rf xwiki_source/user-guide
cp -r ../user-guide xwiki_source/
cp -r ../assets/ xwiki_source/
cp ../index.md xwiki_source/user-guide/

# change style for inline latex math \\( -> {{mathjax}} and \\) -> {{/mathjax}}
find xwiki_source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\\\)=\\)\{\{/mathjax\}\}=g ; s=\\\\(=\{\{mathjax\}\}\\(=g' {} \;

# change style for display latex math \n$$ -> {{mathjax}}\begin{equation*} and $$\n -> \end{equation*}{{/mathjax}}
find xwiki_source/user-guide/ -type f -name "*.md" -exec perl -0777 -pi -e 's/(\s*)\$\$\n\s*\n/\1\\end{equation*}\1\{\{\/mathjax}}\n\n/g; s/\n\n(\s*)\$\$/\n\1\{\{mathjax}}\n\1\\begin{equation*}/g' {} \;

# Replace \\{ with \{ 
find xwiki_source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\\\{=\\\{=g ; s=\\\\}=\\\}=g' {} \;

# Replace \\\\ with \\
find xwiki_source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\\\\\\\=\\\\=g' {} \;

# Replace \_ (used in mkdocs to avoid confusion with italic) with _ for latex/xWiki
find xwiki_source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\_=_=g' {} \;

# # clean
# To be run once the content of the files within xwiki_source/user-guide has been copied (manually) to xWiki
# rm -rf xwiki_source/user-guide
