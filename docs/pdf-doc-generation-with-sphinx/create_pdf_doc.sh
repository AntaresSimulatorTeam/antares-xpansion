#!/bin/bash
# copy reference guide md files and assets
cp -r ../user-guide source/
cp -r ../assets/ source/
cp ../index.md source/user-guide/
# change style for inline latex math \\( -> $ and \\) -> $
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\\\\)=$=g ; s=\\\\(=$=g' {} \;
find source/user-guide/ -type f -name "*.md" -exec sed -i 's=\[!\[Status\].*\]\[.*\]==g; s=!\[C++\]\(.*\)==g; s=!\[Python\]\(.*\)==g' {} \;
# actually make the pdf
sphinx-build -M latexpdf source build
mv build/latex/antaresxpansionuserguide.pdf .
# clean
rm -rf source/user-guide source/assets build
