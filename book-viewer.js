class BookViewer {
    constructor(container, config) {
        this.container = container;
        this.config = config;
        this.currentChapter = -1; // Start at index
        this.currentPage = 0;
        this.isGitHubPages = window.location.hostname.includes('github.io');
        this.basePath = this.isGitHubPages ? '/myJourneyOfBuildingOS' : '';
        this.init();
    }

    init() {
        console.log('Initializing BookViewer');
        this.render();
        this.loadContent();
    }

    async loadContent() {
        try {
            let currentPath;
            if (this.currentChapter === -1) {
                currentPath = this.config.index.pages[this.currentPage].path;
            } else {
                currentPath = this.config.chapters[this.currentChapter].pages[this.currentPage].path;
            }
            
            console.log('Original path:', currentPath);
            
            if (this.isGitHubPages) {
                currentPath = currentPath.startsWith('/') ? currentPath : '/' + currentPath;
            } else {
                currentPath = currentPath.startsWith('./') ? currentPath : './' + currentPath;
            }
            
            const encodedPath = encodeURI(currentPath);
            const response = await fetch(encodedPath);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const text = await response.text();
            const adjustedText = this.isGitHubPages ? 
                text.replace(/\!\[(.*?)\]\((?!http)(.*?)\)/g, '![$1]($2)') 
                : text;
            
            const html = marked.parse(adjustedText);
            document.getElementById('content').innerHTML = html;
            
            this.updateNavigationState();
        } catch (error) {
            console.error('Error loading content:', error);
            document.getElementById('content').innerHTML = `<div class="bg-red-100 border-l-4 border-red-500 text-red-700 p-4" role="alert">
                <p class="font-bold">Error loading content</p>
                <p>${error.message}</p>
            </div>`;
        }
    }

    updateNavigationState() {
        console.log('Current state:', {
            chapter: this.currentChapter,
            page: this.currentPage,
            totalChapters: this.config.chapters.length,
            totalPages: this.currentChapter === -1 ? this.config.index.pages.length : this.config.chapters[this.currentChapter].pages.length
        });
    }

    goToNextPage() {
        console.log('Attempting to go to next page');
        const currentTotalPages = this.currentChapter === -1 ? this.config.index.pages.length : this.config.chapters[this.currentChapter].pages.length;
        
        if (this.currentPage < currentTotalPages - 1) {
            console.log('Moving to next page in current section');
            this.currentPage++;
        } else if (this.currentChapter === -1) { // If we were at the index
            console.log('Moving from index to first chapter');
            this.currentChapter = 0;
            this.currentPage = 0;
        } else if (this.currentChapter < this.config.chapters.length - 1) {
            console.log('Moving to first page of next chapter');
            this.currentChapter++;
            this.currentPage = 0;
        }
        
        this.render();
        this.loadContent();
    }

    goToPrevPage() {
        console.log('Attempting to go to previous page');
        const currentTotalPages = this.currentChapter === -1 ? this.config.index.pages.length : this.config.chapters[this.currentChapter].pages.length;
        
        if (this.currentPage > 0) {
            console.log('Moving to previous page in current section');
            this.currentPage--;
        } else if (this.currentChapter > 0) {
            console.log('Moving to last page of previous chapter');
            this.currentChapter--;
            this.currentPage = this.config.chapters[this.currentChapter].pages.length - 1;
        } else if (this.currentChapter === 0) { // If we're in the first chapter and at the first page
            console.log('Moving back to the index');
            this.currentChapter = -1;
            this.currentPage = 0;
        }
        
        this.render();
        this.loadContent();
    }

    render() {
        let currentContent, chapterTitle, pageTitle;
        if (this.currentChapter === -1) {
            currentContent = this.config.index;
            chapterTitle = currentContent.title;
        } else {
            currentContent = this.config.chapters[this.currentChapter];
            chapterTitle = `Chapter ${this.currentChapter + 1}: ${currentContent.title}`;
        }
        pageTitle = currentContent.pages[this.currentPage].title;
        const isFirstPage = this.currentChapter === -1 && this.currentPage === 0;
        const isLastPage = this.currentChapter === this.config.chapters.length - 1 && this.currentPage === this.config.chapters[this.currentChapter].pages.length - 1;

        this.container.innerHTML = `
            <div class="max-w-4xl mx-auto p-4">
                <nav class="flex justify-between items-center mb-8">
                    <button 
                        onclick="bookViewer.goToPrevPage()"
                        ${isFirstPage ? 'disabled' : ''}
                        class="px-4 py-2 bg-blue-500 text-white rounded ${isFirstPage ? 'opacity-50 cursor-not-allowed' : 'hover:bg-blue-600'}"
                    >
                        ← Previous
                    </button>
                    
                    <div class="text-lg font-semibold text-center">
                        <div>${chapterTitle}</div>
                        <div class="text-sm text-gray-600 mt-1">${pageTitle}</div>
                    </div>
                    
                    <button
                        onclick="bookViewer.goToNextPage()"
                        ${isLastPage ? 'disabled' : ''}
                        class="px-4 py-2 bg-blue-500 text-white rounded ${isLastPage ? 'opacity-50 cursor-not-allowed' : 'hover:bg-blue-600'}"
                    >
                        Next →
                    </button>
                </nav>

                <div id="content" class="prose max-w-none">
                    Loading...
                </div>

                <div class="mt-4 text-sm text-gray-500 text-center">
                    Page ${this.currentPage + 1} of ${currentContent.pages.length}
                </div>
            </div>
        `;
    }
}