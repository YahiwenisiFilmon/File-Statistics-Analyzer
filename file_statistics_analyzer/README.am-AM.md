# የፋይል ሲስተም እና የሎግ መተንተኛ (File System & Log Analyzer)

_Read this in other languages:_
[_አማርኛ_](README.am-AM.md) [_English_](README.md)

![File Analyzer Hero](./file_analyzer_hero.png)
## አጠቃላይ እይታ
ይህ በዘመናዊው C++20 ቋንቋ የተገነባ፣ የማከማቻ አጠቃቀምን እና የሎግ ፋይሎችን በፍጥነት ለመተንተን የሚያገለግል መሳሪያ ነው። ማውጫዎችን (Directories) በመቃኘት ትልልቅ ፋይሎችን ይለያል እንዲሁም የሎግ ፋይሎችን በማንበብ የስህተት መጠኖችን ያሰላል።


## ቅድመ-ሁኔታዎች
### C++ ኮምፓይለር
- **C++20** የሚደግፍ ኮምፓይለር ያስፈልጋል (ለምሳሌ፡ Clang 15+ ወይም GCC 11+)።

## መጫን
### ፩. ፕሮጀክቱን ያውርዱ
```bash
git clone "https://github.com/ElrohiFilmon/File-Stat-Analyzer"
cd file_statistics_analyzer
```

### ፪. በቃኝ ማድረጊያ (Compilation)
`cmake` ከሌለዎት በሚከተለው ትዕዛዝ መገንባት ይችላሉ፦
```bash
clang++ -std=c++20 -Iinclude src/*.cpp -o FileStatAnalyzer
```

## እንዴት ነው አሰራሩ
### የፋይል ሲስተም ፍተሻ
አንድን ማውጫ ለመተንተን፦
```bash
./FileStatAnalyzer fs /የማውጫው/መንገድ
```

### የሎግ ፋይል ፍተሻ
የApache ወይም JSON ሎግ ፋይሎችን ለመተንተን፦
```bash
./FileStatAnalyzer log /የፋይሉ/መንገድ.log
```

### የJSON ውጤት
ውጤቱ በJSON እንዲሆን `--json` የሚለውን ይጨምሩ፦
```bash
./FileStatAnalyzer fs . --json
```

## የኮድ መዋቅር
፩. **FileSystemAnalyzer**: ፋይሎችን በየአይነታቸው ይለያል፣ መጠናቸውን ይደምራል፣ እና ትልልቅ ፋይሎችን ይዘረዝራል።
፪. **LogAnalyzer**: የሎግ ፋይሎችን መስመር በመስመር ያነባል፣ ስህተቶችን ይለያል፣ እና ብዙ ጥያቄ የሚመጣባቸውን አድራሻዎች ይለያል።
፫. **ReportGenerator**: መረጃውን በሰው ሊነበብ በሚችል ጽሁፍ ወይም በኮምፒውተር በሚነበብ JSON ያዘጋጃል።

## ገደቦች እና ማሻሻያዎች
፩. **ጊዜን መለየት**: በአሁኑ ሰዓት የሎግ ፋይሎችን በጊዜ መለየት ገና እየተሻሻለ ነው።
፪. **ጥልቀት**: ማውጫዎችን ሲፈትሽ እስከ ምን ያህል ጥልቀት መሄድ እንዳለበት በCLI ቁጥጥር አልተደረገም።

## የሚጠየቁ ጥያቄዎች
**ጥ፦** በጣም ትልልቅ ፋይሎችን (ከ4GB በላይ) መተንተን ይችላል?
**መልስ፦** አዎ፣ የ64-bit ቁጥሮችን ስለሚጠቀም ማንኛውንም መጠን መተንተን ይችላል።

**ጥ፦** በJSON የማውጣት ጥቅሙ ምንድን ነው?
**መልስ፦** ውጤቱን ለሌሎች የኮምፒውተር ፕሮግራሞች ወይም ለዌብሳይቶች በቀላሉ ለመስጠት ይረዳል።

## ወደፊት የሚጨመሩ ነገሮች
፩. **ግራፍ**: የፋይሎችን መጠን በስዕል የሚያሳይ ግራፍ መጨመር።
፪. **ቀጥታ ክትትል**: ሎግ ፋይሎች እየተጻፉ እያለ በዚያው ቅጽበት መተንተን።

## ዋቢዎች
- [C++ Filesystem Manual](https://en.cppreference.com/w/cpp/filesystem)
- [JSON for Modern C++](https://json.nlohmann.me/)
