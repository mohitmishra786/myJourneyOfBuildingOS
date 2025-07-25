class BookViewer {
    constructor(container, chapters) {
        this.container = container;
        this.chapters = chapters;
        this.currentChapter = 0;
        this.currentPage = 0;
        this.isGitHubPages = window.location.hostname.includes('github.io');
        this.basePath = this.isGitHubPages ? '/myJourneyOfBuildingOS' : '';
        this.sidebarVisible = false;
        this.init();
    }

    init() {
        console.log('Initializing BookViewer');
        console.log('Total chapters:', this.chapters.length);
        
        // Initialize URL routing
        this.initializeRouting();
        
        this.render();
        this.loadContent();
        
        // Set initial URL after content loads
        setTimeout(() => {
            this.updateURL();
        }, 100);
    }

    initializeRouting() {
        // Handle browser back/forward buttons
        window.addEventListener('popstate', (event) => {
            if (event.state) {
                this.currentChapter = event.state.chapter;
                this.currentPage = event.state.page;
                this.render();
                this.loadContent();
            } else {
                this.loadFromURL();
            }
        });

        // Load initial page from URL
        this.loadFromURL();
    }

    loadFromURL() {
        // Check for redirected path from 404.html
        let path = sessionStorage.getItem('redirectPath');
        if (path) {
            sessionStorage.removeItem('redirectPath');
        } else {
            path = window.location.pathname.replace(this.basePath, '').replace(/^\//, '');
        }
        
        console.log('🔍 Loading from URL path:', path);
        console.log('🔍 Base path:', this.basePath);
        console.log('🔍 Full pathname:', window.location.pathname);
        
        if (!path || path === '' || path === 'index.html') {
            console.log('📍 Default to first page (empty path)');
            this.currentChapter = 0;
            this.currentPage = 0;
            return;
        }

        // Enhanced matching with detailed logging
        console.log('🔍 Starting page matching for path:', path);
        let allPages = [];
        
        for (let chapterIndex = 0; chapterIndex < this.chapters.length; chapterIndex++) {
            const chapter = this.chapters[chapterIndex];
            console.log(`📖 Chapter ${chapterIndex}: ${chapter.title}`);
            
            for (let pageIndex = 0; pageIndex < chapter.pages.length; pageIndex++) {
                const page = chapter.pages[pageIndex];
                const pageSlug = this.createSlugFromPath(page.path);
                
                // Store all pages for debugging
                allPages.push({
                    chapterIndex,
                    pageIndex,
                    title: page.title,
                    path: page.path,
                    slug: pageSlug
                });
                
                console.log(`   📄 Page ${pageIndex}: "${page.title}"`);
                console.log(`      File: ${page.path}`);
                console.log(`      Slug: ${pageSlug}`);
                console.log(`      Match? ${path === pageSlug ? '✅' : '❌'}`);
                
                if (path === pageSlug) {
                    console.log(`🎯 FOUND MATCH! Chapter ${chapterIndex}, Page ${pageIndex}: ${page.title}`);
                    this.currentChapter = chapterIndex;
                    this.currentPage = pageIndex;
                    return;
                }
                
                // Also try partial matching for better debugging
                if (pageSlug.includes(path) || path.includes(pageSlug)) {
                    console.log(`🔄 Partial match found: "${pageSlug}" contains or is contained in "${path}"`);
                }
            }
        }
        
        console.log('❌ No exact match found!');
        console.log('📝 All available pages:');
        allPages.forEach(page => {
            console.log(`   ${page.slug} -> ${page.title} (${page.path})`);
        });
        
        // Try fuzzy matching as fallback
        const fuzzyMatch = this.findFuzzyMatch(path, allPages);
        if (fuzzyMatch) {
            console.log(`🔄 Using fuzzy match: ${fuzzyMatch.title}`);
            this.currentChapter = fuzzyMatch.chapterIndex;
            this.currentPage = fuzzyMatch.pageIndex;
            return;
        }
        
        console.log('📍 No matches found, defaulting to first page');
        this.currentChapter = 0;
        this.currentPage = 0;
    }

    createSlugFromPath(filePath) {
        // Extract filename without extension
        const filename = filePath.split('/').pop().replace('.md', '');
        
        // Convert to URL-friendly slug
        const slug = filename
            .toLowerCase()
            .replace(/[^a-z0-9]+/g, '-')
            .replace(/^-+|-+$/g, '');
            
        console.log(`🔧 Slug creation: "${filename}" -> "${slug}"`);
        return slug;
    }

    findFuzzyMatch(targetPath, allPages) {
        console.log('🔍 Starting fuzzy matching for:', targetPath);
        
        // Try different matching strategies
        for (let page of allPages) {
            // Strategy 1: Check if the target path contains the page slug
            if (targetPath.includes(page.slug)) {
                console.log(`🎯 Fuzzy match (contains): ${page.slug} found in ${targetPath}`);
                return page;
            }
            
            // Strategy 2: Check if the page slug contains the target path
            if (page.slug.includes(targetPath)) {
                console.log(`🎯 Fuzzy match (contained): ${targetPath} found in ${page.slug}`);
                return page;
            }
            
            // Strategy 3: Remove common words and try matching
            const simplifiedTarget = targetPath.replace(/-?(and|or|the|of|in|to|into|a|an)(-|$)/g, '-').replace(/-+/g, '-').replace(/^-+|-+$/g, '');
            const simplifiedSlug = page.slug.replace(/-?(and|or|the|of|in|to|into|a|an)(-|$)/g, '-').replace(/-+/g, '-').replace(/^-+|-+$/g, '');
            
            if (simplifiedTarget === simplifiedSlug) {
                console.log(`🎯 Fuzzy match (simplified): "${simplifiedTarget}" matches "${simplifiedSlug}"`);
                return page;
            }
        }
        
        // Strategy 4: Try word-by-word matching
        const targetWords = targetPath.split('-').filter(word => word.length > 2);
        
        for (let page of allPages) {
            const pageWords = page.slug.split('-').filter(word => word.length > 2);
            const matchedWords = targetWords.filter(word => pageWords.includes(word));
            
            // If more than 70% of words match, consider it a match
            if (matchedWords.length / targetWords.length > 0.7) {
                console.log(`🎯 Fuzzy match (word overlap): ${matchedWords.length}/${targetWords.length} words match`);
                return page;
            }
        }
        
        console.log('❌ No fuzzy matches found');
        return null;
    }

    updateURL() {
        const currentPage = this.chapters[this.currentChapter].pages[this.currentPage];
        const slug = this.createSlugFromPath(currentPage.path);
        
        // For GitHub Pages, include the repository path
        const newURL = this.isGitHubPages ? 
            `/myJourneyOfBuildingOS/${slug}` : 
            `/${slug}`;
        
        const state = {
            chapter: this.currentChapter,
            page: this.currentPage,
            title: currentPage.title
        };
        
        // Update browser URL and title
        document.title = `${currentPage.title} - OS Learning Journey`;
        
        // Update URL without page reload
        if (window.location.pathname !== newURL) {
            history.pushState(state, currentPage.title, newURL);
            console.log('URL updated to:', newURL);
        }
    }

    async loadContent() {
        try {
            let currentPath = this.chapters[this.currentChapter].pages[this.currentPage].path;
            console.log('Original path:', currentPath);
            
            // Fix path resolution
            if (this.isGitHubPages) {
                currentPath = this.basePath + '/' + currentPath;
            } else {
                // For local development, use relative path
                currentPath = currentPath;
            }
            
            console.log('Loading content from:', currentPath);
            
            const response = await fetch(currentPath);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const text = await response.text();
            console.log('Content loaded, length:', text.length);
            
            // Process PlantUML diagrams before parsing markdown
            let processedText = await this.processPlantUML(text);
            
            // Fix local image paths for GitHub Pages
            processedText = this.processLocalImages(processedText);
            
            const html = marked.parse(processedText);
            
            document.getElementById('content').innerHTML = html;
            
            // Apply syntax highlighting and other enhancements
            this.enhanceContent();
            
            // Update URL to reflect current page
            this.updateURL();
            
        } catch (error) {
            console.error('Error loading content:', error);
            document.getElementById('content').innerHTML = `
                <div class="text-center py-12">
                    <div class="text-red-500 dark:text-red-400 text-lg font-medium mb-4">
                        Failed to load content
                    </div>
                    <div class="text-gray-600 dark:text-gray-300 text-sm">
                        ${error.message}
                    </div>
                </div>
            `;
        }
    }

    async processPlantUML(text) {
        // Use the working PlantUML.com service instead of Kroki
        return text.replace(/```plantuml\n([\s\S]*?)\n```/g, (match, diagram) => {
            const cleanDiagram = diagram.trim();
            
            // Add @startuml and @enduml if not present
            let uml = cleanDiagram;
            if (!uml.startsWith('@startuml')) {
                uml = '@startuml\n' + uml;
            }
            if (!uml.endsWith('@enduml')) {
                uml = uml + '\n@enduml';
            }
            
            // Use hex encoding for PlantUML.com which was working
            const hexEncoded = this.hexEncode(uml);
            const plantUMLUrl = `https://www.plantuml.com/plantuml/svg/~h${hexEncoded}`;
            
            return `<div class="plantuml-container my-8 bg-white p-6 rounded-lg border border-gray-200 shadow-sm dark:bg-gray-800 dark:border-gray-600">
    <img src="${plantUMLUrl}" alt="PlantUML Diagram" class="max-w-full h-auto mx-auto" />
</div>`;
        });
    }

    // Remove the problematic encodeForKroki method and use the working hex encode
    hexEncode(text) {
        let result = '';
        for (let i = 0; i < text.length; i++) {
            result += text.charCodeAt(i).toString(16).padStart(2, '0');
        }
        return result;
    }

    // Create PlantUML URL using a working encoding method
    createPlantUMLUrl(plantuml_text) {
        // This method is now consolidated into processPlantUML
        // Add @startuml and @enduml if not present
        let uml = plantuml_text.trim();
        if (!uml.startsWith('@startuml')) {
            uml = '@startuml\n' + uml;
        }
        if (!uml.endsWith('@enduml')) {
            uml = uml + '\n@enduml';
        }
        
        const hexEncoded = this.hexEncode(uml);
        return `https://www.plantuml.com/plantuml/svg/~h${hexEncoded}`;
    }

    // Create a fallback SVG when PlantUML fails
    createPlantUMLFallback(originalText) {
        const lines = originalText.split('\n').slice(0, 5);
        const svgContent = `
            <svg width="500" height="300" xmlns="http://www.w3.org/2000/svg">
                <defs>
                    <pattern id="grid" width="20" height="20" patternUnits="userSpaceOnUse">
                        <path d="M 20 0 L 0 0 0 20" fill="none" stroke="#e5e7eb" stroke-width="1"/>
                    </pattern>
                </defs>
                <rect width="100%" height="100%" fill="url(#grid)"/>
                <rect x="10" y="10" width="480" height="280" fill="#f8f9fa" stroke="#dee2e6" stroke-width="2" rx="8"/>
                <text x="250" y="50" text-anchor="middle" fill="#495057" font-family="Inter, sans-serif" font-size="18" font-weight="600">
                    📊 PlantUML Diagram
                </text>
                <text x="250" y="80" text-anchor="middle" fill="#6c757d" font-family="Inter, sans-serif" font-size="14">
                    Diagram rendering is temporarily unavailable
                </text>
                <foreignObject x="30" y="120" width="440" height="120">
                    <div xmlns="http://www.w3.org/1999/xhtml" style="font-family: 'JetBrains Mono', monospace; font-size: 12px; color: #495057; background: #f1f3f4; padding: 15px; border-radius: 6px; max-height: 100px; overflow: hidden;">
                        <strong>Source Preview:</strong><br/>
                        ${lines.slice(0, 4).map(line => line.substring(0, 50) + (line.length > 50 ? '...' : '')).join('<br/>')}
                        ${lines.length > 4 ? '<br/>...' : ''}
                    </div>
                </foreignObject>
                <text x="250" y="270" text-anchor="middle" fill="#007bff" font-family="Inter, sans-serif" font-size="12">
                    💡 Click details below to view full source code
                </text>
            </svg>
        `;
        return `data:image/svg+xml;base64,${btoa(svgContent)}`;
    }



    processLocalImages(text) {
        // Fix local image paths for GitHub Pages compatibility
        if (this.isGitHubPages) {
            // Replace image paths that start with /images/ to include the repository path
            text = text.replace(/!\[([^\]]*)\]\(\/images\//g, `![$1](${this.basePath}/images/`);
            
            // Also handle relative image paths
            text = text.replace(/!\[([^\]]*)\]\(images\//g, `![$1](${this.basePath}/images/`);
        }
        
        return text;
    }

    enhanceContent() {
        // Apply Prism.js syntax highlighting
        if (window.Prism) {
            window.Prism.highlightAll();
        }

        // Add copy buttons to code blocks
        const codeBlocks = document.querySelectorAll('pre code');
        codeBlocks.forEach(block => {
            const pre = block.parentNode;
            if (!pre.querySelector('.copy-btn')) {
                const copyBtn = document.createElement('button');
                copyBtn.className = 'copy-btn absolute top-2 right-2 px-2 py-1 text-xs bg-gray-800 dark:bg-gray-700 text-white rounded opacity-0 hover:opacity-100 transition-all duration-200 hover:bg-gray-700 dark:hover:bg-gray-600 font-mono';
                copyBtn.textContent = 'Copy';
                copyBtn.onclick = () => {
                    navigator.clipboard.writeText(block.textContent);
                    copyBtn.textContent = 'Copied!';
                    copyBtn.classList.add('bg-green-600');
                    setTimeout(() => {
                        copyBtn.textContent = 'Copy';
                        copyBtn.classList.remove('bg-green-600');
                    }, 2000);
                };
                pre.style.position = 'relative';
                pre.appendChild(copyBtn);
            }
        });

        // Add smooth scrolling to headers
        const headers = document.querySelectorAll('h1, h2, h3, h4, h5, h6');
        headers.forEach(header => {
            header.style.scrollMarginTop = '100px';
            
            // Add anchor links to headers
            if (!header.querySelector('.header-link')) {
                const anchor = document.createElement('a');
                anchor.className = 'header-link opacity-0 hover:opacity-100 transition-opacity duration-200 ml-2 text-blue-500 hover:text-blue-600 text-sm';
                anchor.innerHTML = '#';
                anchor.href = '#' + (header.id || header.textContent.toLowerCase().replace(/\s+/g, '-'));
                anchor.title = 'Link to this section';
                header.appendChild(anchor);
                
                // Show anchor on header hover
                header.addEventListener('mouseenter', () => {
                    anchor.classList.remove('opacity-0');
                    anchor.classList.add('opacity-70');
                });
                header.addEventListener('mouseleave', () => {
                    anchor.classList.add('opacity-0');
                    anchor.classList.remove('opacity-70');
                });
            }
        });

        // Add hover effects to tables
        const tables = document.querySelectorAll('table');
        tables.forEach(table => {
            table.classList.add('table-auto', 'border-collapse', 'bg-white', 'dark:bg-gray-800', 'shadow-sm', 'rounded-lg', 'overflow-hidden');
        });

        // Enhance blockquotes
        const blockquotes = document.querySelectorAll('blockquote');
        blockquotes.forEach(blockquote => {
            if (!blockquote.querySelector('.blockquote-icon')) {
                blockquote.classList.add('relative');
            }
        });

        // Add reading time estimate
        const content = document.getElementById('content');
        if (content && !content.querySelector('.reading-time')) {
            const text = content.textContent;
            const wordsPerMinute = 200;
            const words = text.trim().split(/\s+/).length;
            const readingTime = Math.ceil(words / wordsPerMinute);
            
            const readingTimeElement = document.createElement('div');
            readingTimeElement.className = 'reading-time text-sm text-gray-500 dark:text-gray-400 mb-8 flex items-center';
            readingTimeElement.innerHTML = `
                <svg class="w-4 h-4 mr-2" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" />
                </svg>
                ${readingTime} min read • ${words} words
            `;
            
            const firstChild = content.firstElementChild;
            if (firstChild) {
                content.insertBefore(readingTimeElement, firstChild);
            }
        }
    }

    updateNavigationState() {
        console.log('Current state:', {
            chapter: this.currentChapter,
            page: this.currentPage,
            totalChapters: this.chapters.length,
            totalPages: this.chapters[this.currentChapter].pages.length
        });

        // Update progress bar
        const totalPages = this.chapters.reduce((sum, chapter) => sum + chapter.pages.length, 0);
        let currentPageGlobal = 0;
        for (let i = 0; i < this.currentChapter; i++) {
            currentPageGlobal += this.chapters[i].pages.length;
        }
        currentPageGlobal += this.currentPage + 1;
        
        const progressPercent = (currentPageGlobal / totalPages) * 100;
        const progressBar = document.getElementById('progress-bar');
        if (progressBar) {
            progressBar.style.width = `${progressPercent}%`;
        }
    }

    toggleSidebar() {
        this.sidebarVisible = !this.sidebarVisible;
        const sidebar = document.getElementById('sidebar');
        const overlay = document.getElementById('sidebar-overlay');
        const mainContent = document.getElementById('main-content');
        
        if (this.sidebarVisible) {
            sidebar.classList.remove('-translate-x-full');
            overlay.classList.remove('hidden');
        } else {
            sidebar.classList.add('-translate-x-full');
            overlay.classList.add('hidden');
        }
    }

    goToPage(chapterIndex, pageIndex) {
        this.currentChapter = chapterIndex;
        this.currentPage = pageIndex;
        
        if (this.sidebarVisible) {
            this.toggleSidebar(); // Close sidebar after navigation on mobile
        }
        this.render();
        this.loadContent();
    }

    goToNextPage() {
        const currentChapter = this.chapters[this.currentChapter];
        
        if (this.currentPage < currentChapter.pages.length - 1) {
            this.currentPage++;
        } else if (this.currentChapter < this.chapters.length - 1) {
            this.currentChapter++;
            this.currentPage = 0;
        }
        
        if (this.sidebarVisible) {
            this.toggleSidebar(); // Close sidebar after navigation on mobile
        }
        this.render();
        this.loadContent();
    }

    goToPrevPage() {
        if (this.currentPage > 0) {
            this.currentPage--;
        } else if (this.currentChapter > 0) {
            this.currentChapter--;
            this.currentPage = this.chapters[this.currentChapter].pages.length - 1;
        }
        
        if (this.sidebarVisible) {
            this.toggleSidebar(); // Close sidebar after navigation on mobile
        }
        this.render();
        this.loadContent();
    }

    generateTableOfContents() {
        return this.chapters.map((chapter, chapterIndex) => {
            const pages = chapter.pages.map((page, pageIndex) => {
                const isActive = chapterIndex === this.currentChapter && pageIndex === this.currentPage;
                return `
                    <li>
                        <button 
                            onclick="bookViewer.goToPage(${chapterIndex}, ${pageIndex})"
                            class="block w-full text-left px-3 py-2 text-sm rounded-lg transition-all duration-150 ${
                                isActive 
                                    ? 'bg-blue-50 dark:bg-blue-900/30 text-blue-700 dark:text-blue-300 font-medium border-l-2 border-blue-500 dark:border-blue-400' 
                                    : 'text-gray-700 dark:text-gray-300 hover:bg-gray-50 dark:hover:bg-gray-700 hover:text-gray-900 dark:hover:text-white'
                            }"
                        >
                            ${page.title}
                        </button>
                    </li>
                `;
            }).join('');

            return `
                <div class="mb-6">
                    <h3 class="font-semibold text-gray-900 dark:text-white mb-3 px-3 text-sm uppercase tracking-wider">${chapter.title}</h3>
                    <ul class="space-y-1">
                        ${pages}
                    </ul>
                </div>
            `;
        }).join('');
    }

    render() {
        const currentChapter = this.chapters[this.currentChapter];
        const currentPage = currentChapter.pages[this.currentPage];
        const isFirstPage = this.currentChapter === 0 && this.currentPage === 0;
        const isLastPage = this.currentChapter === this.chapters.length - 1 && 
                          this.currentPage === this.chapters[this.currentChapter].pages.length - 1;

        this.container.innerHTML = `
            <div class="flex h-screen bg-gray-50 dark:bg-gray-900">
                <!-- Sidebar Overlay for Mobile -->
                <div id="sidebar-overlay" class="fixed inset-0 bg-black bg-opacity-50 z-40 lg:hidden hidden" onclick="bookViewer.toggleSidebar()"></div>
                
                <!-- Sidebar -->
                <div id="sidebar" class="fixed lg:relative left-0 top-0 bottom-0 w-80 bg-white dark:bg-gray-800 shadow-xl transform -translate-x-full lg:transform-none transition-transform duration-300 ease-in-out z-50 lg:border-r lg:border-gray-200 dark:lg:border-gray-600 lg:shadow-none flex-shrink-0 lg:flex lg:flex-col">
                    <!-- Mobile header -->
                    <div class="flex items-center justify-between p-4 border-b border-gray-200 dark:border-gray-600 lg:hidden">
                        <h2 class="text-lg font-semibold text-gray-900 dark:text-white">Table of Contents</h2>
                        <button onclick="bookViewer.toggleSidebar()" class="p-2 rounded-md text-gray-400 dark:text-gray-300 hover:text-gray-500 dark:hover:text-gray-200 hover:bg-gray-100 dark:hover:bg-gray-700">
                            <svg class="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
                            </svg>
                        </button>
                    </div>
                    
                    <!-- Sidebar content -->
                    <div class="flex-1 overflow-y-auto">
                        <div class="p-4">
                            <!-- Desktop header -->
                            <div class="hidden lg:block mb-8 pb-6 border-b border-gray-200 dark:border-gray-600">
                                <h1 class="text-xl font-bold text-gray-900 dark:text-white">OS Learning Journey</h1>
                                <p class="text-sm text-gray-600 dark:text-gray-300 mt-2">Operating Systems Architecture & Design</p>
                            </div>
                            ${this.generateTableOfContents()}
                        </div>
                    </div>
                </div>

                <!-- Main Content Area -->
                <div id="main-content" class="flex-1 flex flex-col min-h-0 overflow-hidden">
                    <!-- Header -->
                    <header class="bg-white dark:bg-gray-800 shadow-sm border-b border-gray-200 dark:border-gray-600 flex-shrink-0">
                        <div class="px-4 lg:px-8 py-4">
                            <div class="flex items-center w-full">
                                <!-- Left Side: Hamburger + Title -->
                                <div class="flex items-center space-x-4 flex-1">
                                    <button 
                                        onclick="bookViewer.toggleSidebar()"
                                        class="p-2 rounded-md text-gray-400 hover:text-gray-500 hover:bg-gray-100 dark:hover:bg-gray-700 lg:hidden"
                                    >
                                        <svg class="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16" />
                                        </svg>
                                    </button>
                                    <div>
                                        <h1 class="text-xl font-semibold text-gray-900 dark:text-white">${currentChapter.title}</h1>
                                        <p class="text-sm text-gray-600 dark:text-gray-300 mt-1">${currentPage.title}</p>
                                    </div>
                                </div>
                                
                                <!-- Right Side: Navigation buttons at absolute right edge -->
                                <div class="flex items-center space-x-2 flex-shrink-0">
                                    <!-- Dark Mode Toggle -->
                                    <button 
                                        onclick="toggleDarkMode()"
                                        class="dark-mode-toggle focus:outline-none focus:ring-2 focus:ring-blue-500 focus:ring-offset-2 dark:focus:ring-offset-gray-800"
                                        title="Toggle dark mode"
                                    ></button>
                                    
                                    <!-- Navigation buttons -->
                                    <button 
                                        onclick="bookViewer.goToPrevPage()"
                                        ${isFirstPage ? 'disabled' : ''}
                                        class="inline-flex items-center px-3 py-2 border border-gray-300 dark:border-gray-600 shadow-sm text-sm font-medium rounded-md text-gray-700 dark:text-gray-200 bg-white dark:bg-gray-700 hover:bg-gray-50 dark:hover:bg-gray-600 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                                        title="Previous page"
                                    >
                                        <svg class="h-4 w-4 mr-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 19l-7-7 7-7" />
                                        </svg>
                                        Previous
                                    </button>
                                    
                                    <button
                                        onclick="bookViewer.goToNextPage()"
                                        ${isLastPage ? 'disabled' : ''}
                                        class="inline-flex items-center px-3 py-2 border border-transparent text-sm font-medium rounded-md text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                                        title="Next page"
                                    >
                                        Next
                                        <svg class="h-4 w-4 ml-1" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7" />
                                        </svg>
                                    </button>
                                </div>
                            </div>
                            
                            <!-- Progress Bar -->
                            <div class="mt-4">
                                <div class="bg-gray-200 dark:bg-gray-700 rounded-full h-2">
                                    <div id="progress-bar" class="bg-blue-600 h-2 rounded-full transition-all duration-500" style="width: 0%"></div>
                                </div>
                            </div>
                        </div>
                    </header>

                    <!-- Content Area -->
                    <main class="flex-1 overflow-y-auto">
                        <div class="w-full px-4 lg:px-8 py-8">
                            <article id="content" class="prose prose-lg dark:prose-dark max-w-none bg-white dark:bg-gray-800 rounded-lg shadow-sm p-8 lg:p-12">
                                <div class="animate-pulse space-y-4">
                                    <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-3/4"></div>
                                    <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-1/2"></div>
                                    <div class="h-4 bg-gray-200 dark:bg-gray-700 rounded w-2/3"></div>
                                </div>
                            </article>
                            
                            <!-- Page Navigation Footer -->
                            <footer class="mt-8 pt-8 border-t border-gray-200 dark:border-gray-700">
                                <div class="flex justify-between items-center w-full">
                                    <div class="text-sm text-gray-500 dark:text-gray-400">
                                        Page ${this.currentPage + 1} of ${currentChapter.pages.length} in this chapter
                                    </div>
                                    
                                    <!-- Only Next button positioned at right extreme -->
                                    <div class="flex justify-end">
                                        <button
                                            onclick="bookViewer.goToNextPage()"
                                            ${isLastPage ? 'disabled' : ''}
                                            class="inline-flex items-center px-4 py-2 border border-transparent text-sm font-medium rounded-md text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                                            title="Next page"
                                        >
                                            ${this.getNextPageTitle()}
                                            <svg class="h-4 w-4 ml-2" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7" />
                                            </svg>
                                        </button>
                                    </div>
                                </div>
                            </footer>
                        </div>
                    </main>
                </div>
            </div>
        `;

        // Update progress bar
        const totalPages = this.chapters.reduce((sum, chapter) => sum + chapter.pages.length, 0);
        const currentPageIndex = this.chapters.slice(0, this.currentChapter)
            .reduce((sum, chapter) => sum + chapter.pages.length, 0) + this.currentPage + 1;
        const progress = (currentPageIndex / totalPages) * 100;
        
        setTimeout(() => {
            const progressBar = document.getElementById('progress-bar');
            if (progressBar) {
                progressBar.style.width = `${progress}%`;
            }
        }, 100);
    }

    getPreviousPageTitle() {
        if (this.currentPage > 0) {
            return this.chapters[this.currentChapter].pages[this.currentPage - 1].title;
        } else if (this.currentChapter > 0) {
            const prevChapter = this.chapters[this.currentChapter - 1];
            return prevChapter.pages[prevChapter.pages.length - 1].title;
        }
        return '';
    }

    getNextPageTitle() {
        const currentChapterPages = this.chapters[this.currentChapter].pages.length;
        
        if (this.currentPage < currentChapterPages - 1) {
            return this.chapters[this.currentChapter].pages[this.currentPage + 1].title;
        } else if (this.currentChapter < this.chapters.length - 1) {
            return this.chapters[this.currentChapter + 1].pages[0].title;
        }
        return '';
    }
}