#!/usr/bin/awk -f

# -----------------------------------------------------------------------------
# Fenrir Chess Engine Coverage Cleanup Script (Simplified)
# -----------------------------------------------------------------------------
# This AWK script removes DA lines for truly unreachable code (===== in gcov)
# -----------------------------------------------------------------------------

BEGIN {
    print "🔧 Cleaning truly unreachable code markers..." > "/dev/stderr"
    
    # Build list of unreachable lines from gcov files
    cmd = "find .debug -name '*.gcov' -exec grep -H '^    =====:' {} \\;"
    while ((cmd | getline line) > 0) {
        # Parse: .debug/filename.cpp.gcov:    =====:   71:    }
        split(line, parts, ":")
        gcov_file = parts[1]
        line_num = parts[3]
        
        # Extract filename from gcov path: .debug/engine.cpp.gcov -> engine.cpp
        gsub(/^\.debug\//, "", gcov_file)
        gsub(/\.gcov$/, "", gcov_file)
        
        # Trim whitespace from line number
        gsub(/^[ \t]+/, "", line_num)
        gsub(/[ \t]+$/, "", line_num)
        
        unreachable_key = gcov_file ":" line_num
        unreachable[unreachable_key] = 1
        print "  Found unreachable line " line_num " in " gcov_file > "/dev/stderr"
    }
    close(cmd)
}

/^SF:/ {
    current_file = $0
    gsub(/^SF:.*\//, "", current_file)
    print
    next
}

/^DA:[0-9]+,0$/ {
    line_num = $0
    gsub(/^DA:/, "", line_num)
    gsub(/,0$/, "", line_num)
    
    key = current_file ":" line_num
    if (unreachable[key]) {
        print "  Removing unreachable line " line_num " from " current_file > "/dev/stderr"
        next
    }
    
    print
    next
}

{ print }
